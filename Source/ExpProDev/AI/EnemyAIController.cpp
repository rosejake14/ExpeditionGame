// No Rights Reserved @ Team Expedition

#include "AI/EnemyAIController.h"
#include "AI/EnemyCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"

AEnemyAIController::AEnemyAIController()
{
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	HomeLocation = InPawn->GetActorLocation();

	// Apply per-character perception values so designers can tune them per-Blueprint
	if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(InPawn))
	{
		SightConfig->SightRadius                    = Enemy->SightRadius;
		SightConfig->LoseSightRadius                = Enemy->LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees   = Enemy->PeripheralVisionAngle;
		SightConfig->SetMaxAge(10.f);
		SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

		AIPerception->ConfigureSense(*SightConfig);
		AIPerception->RequestStimuliListenerUpdate();
	}

	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);

	SetState(EEnemyState::Roam);
}

void AEnemyAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(RoamWaitHandle);
	GetWorldTimerManager().ClearTimer(AttackChaseHandle);
	AIPerception->OnTargetPerceptionUpdated.RemoveDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
	Super::OnUnPossess();
}

// ── State Machine ─────────────────────────────────────────────────────────────

void AEnemyAIController::SetState(EEnemyState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;

	GetWorldTimerManager().ClearTimer(RoamWaitHandle);
	GetWorldTimerManager().ClearTimer(AttackChaseHandle);
	StopMovement();

	switch (CurrentState)
	{
	case EEnemyState::Roam:
		PickNewRoamTarget();
		break;

	case EEnemyState::Attack:
		BeginChase();
		break;
	}
}

// ── Roam ──────────────────────────────────────────────────────────────────────

void AEnemyAIController::PickNewRoamTarget()
{
	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
	if (!Enemy) return;

	const float Angle    = FMath::FRandRange(0.f, 2.f * PI);
	const float Distance = FMath::FRandRange(0.f, Enemy->RoamRadius);
	const FVector Target = HomeLocation + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);

	MoveToLocation(Target, 50.f);
}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (CurrentState != EEnemyState::Roam) return;

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
	if (!Enemy) return;

	const float Wait = FMath::FRandRange(Enemy->RoamWaitMin, Enemy->RoamWaitMax);
	GetWorldTimerManager().SetTimer(RoamWaitHandle, this, &AEnemyAIController::PickNewRoamTarget, Wait, false);
}

// ── Attack ────────────────────────────────────────────────────────────────────

void AEnemyAIController::BeginChase()
{
	// Issue an immediate move, then repeat on a short interval so we track a moving player
	if (AttackTarget.IsValid())
	{
		MoveToActor(AttackTarget.Get(), 100.f);
	}

	GetWorldTimerManager().SetTimer(AttackChaseHandle, this, &AEnemyAIController::BeginChase, 0.3f, false);
}

// ── Perception ────────────────────────────────────────────────────────────────

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		AttackTarget = Actor;
		SetState(EEnemyState::Attack);
	}
	else
	{
		// Lost sight — return home and resume roaming
		AttackTarget = nullptr;
		CurrentState = EEnemyState::Roam; // set directly so SetState doesn't skip same-state check
		GetWorldTimerManager().ClearTimer(AttackChaseHandle);
		MoveToLocation(HomeLocation, 50.f);
		// Once the move home completes, OnMoveCompleted resumes normal roaming
	}
}
