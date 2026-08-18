// No Rights Reserved @ Team Expedition

#include "AI/EnemyAIController.h"
#include "AI/EnemyCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIController::AEnemyAIController()
{
	SightConfig  = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &AEnemyAIController::OnPerceptionForgotten);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	HomeLocation = InPawn->GetActorLocation();

	if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(InPawn))
	{
		SightConfig->SightRadius                  = Enemy->SightRadius;
		SightConfig->LoseSightRadius              = Enemy->LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = Enemy->PeripheralVisionAngle;
		// TECH_DEBT(TD-ARCH-16): hardcoded stimulus age while every neighbouring value is designer-
		// facing on AEnemyCharacter — expose it there too.
		SightConfig->SetMaxAge(10.f);
		SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
		AIPerception->ConfigureSense(*SightConfig);
		AIPerception->RequestStimuliListenerUpdate();
	}

	CurrentState = EEnemyAIState::Roaming;
	PickRoamPoint();
}

void AEnemyAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(RoamTimerHandle);
	GetWorldTimerManager().ClearTimer(ChaseRefreshHandle);
	Super::OnUnPossess();
}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (CurrentState == EEnemyAIState::Roaming)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
		float WaitTime = Enemy
			? FMath::RandRange(Enemy->RoamWaitMin, Enemy->RoamWaitMax)
			: 3.f;

		GetWorldTimerManager().SetTimer(RoamTimerHandle, this, &AEnemyAIController::PickRoamPoint, WaitTime, false);
	}
}

void AEnemyAIController::PickRoamPoint()
{
	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
	float RoamRadius = Enemy ? Enemy->RoamRadius : 1000.f;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation NavLocation;

	if (NavSys && NavSys->GetRandomReachablePointInRadius(HomeLocation, RoamRadius, NavLocation))
	{
		MoveToLocation(NavLocation.Location, 50.f);
	}
	else
	{
		// Nav not ready yet — retry after a short wait
		GetWorldTimerManager().SetTimer(RoamTimerHandle, this, &AEnemyAIController::PickRoamPoint, 1.f, false);
	}
}

void AEnemyAIController::StartChasing(AActor* Target)
{
	CurrentState = EEnemyAIState::Chasing;
	TargetActor  = Target;
	GetWorldTimerManager().ClearTimer(RoamTimerHandle);
	MoveToActor(Target, 100.f);
	GetWorldTimerManager().SetTimer(ChaseRefreshHandle, this, &AEnemyAIController::RefreshChase, ChaseRefreshInterval, true);
}

void AEnemyAIController::StopChasing()
{
	GetWorldTimerManager().ClearTimer(ChaseRefreshHandle);
	GetWorldTimerManager().ClearTimer(AttackCooldownHandle);
	bCanAttack   = true;
	CurrentState = EEnemyAIState::Roaming;
	TargetActor  = nullptr;

	if (ACharacter* EnemyPawn = Cast<ACharacter>(GetPawn()))
		EnemyPawn->StopAnimMontage();

	StopMovement();
	PickRoamPoint();
}

void AEnemyAIController::RefreshChase()
{
	if (!IsValid(TargetActor))
	{
		StopChasing();
		return;
	}

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
	// TECH_DEBT(TD-ARCH-15): every tuning default is duplicated here as a magic-number fallback
	// (150.f range, 10.f damage, 1.5f cooldown, 1000.f roam radius, 3.f roam wait) alongside the
	// real EditAnywhere defaults on AEnemyCharacter. Two sources of truth that already disagree.
	float Range = Enemy ? Enemy->AttackRange : 150.f;

	// TECH_DEBT(TD-BUG-6): GetPawn() is dereferenced unguarded — this fires on a repeating timer
	// that is not cleared when the pawn is destroyed mid-chase.
	float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());

	if (DistToTarget <= Range)
	{
		StopMovement();
		PerformAttack();
	}
	else
	{
		MoveToActor(TargetActor, 5.f, true, true, false);
	}
}

void AEnemyAIController::PerformAttack()
{
	if (!bCanAttack || !IsValid(TargetActor)) return;

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
	float Damage   = Enemy ? Enemy->AttackDamage   : 10.f;
	float Cooldown = Enemy ? Enemy->AttackCooldown  : 1.5f;

	if (Enemy && Enemy->AttackMontage)
		Enemy->PlayAnimMontage(Enemy->AttackMontage);

	// TECH_DEBT(TD-BUG-7): damage lands on the same frame the montage starts, with no line-of-sight
	// check and no re-test of range at the moment of impact. Enemies therefore hit through walls
	// and land damage before the swing is visible. Should be driven by an anim notify + LOS trace.
	UGameplayStatics::ApplyDamage(TargetActor, Damage, this, GetPawn(), UDamageType::StaticClass());

	bCanAttack = false;
	GetWorldTimerManager().SetTimer(AttackCooldownHandle, this, &AEnemyAIController::ResetAttackCooldown, Cooldown, false);
}

void AEnemyAIController::ResetAttackCooldown()
{
	bCanAttack = true;
}

void AEnemyAIController::ForceChase(AActor* Target)
{
	if (Target && Target != TargetActor)
		StartChasing(Target);
}

void AEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// TECH_DEBT(TD-ARCH-16): target selection is a hardcoded "Player" actor-tag string literal. It
	// fails silently if the tag is missing from BP_PlayerCharacter and can't express factions.
	if (!Actor || !Actor->ActorHasTag(FName("Player"))) return;

	if (Stimulus.WasSuccessfullySensed())
		StartChasing(Actor);
}

void AEnemyAIController::OnPerceptionForgotten(AActor* Actor)
{
	if (Actor == TargetActor)
		StopChasing();
}
