// No Rights Reserved @ Team Expedition


#include "Gamemode/DefaultGameMode.h"

#include "Character/PlayerCharacter.h"
#include "Core/ExpProGameInstance.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroDefinition.h"
#include "Hero/HeroRegistry.h"
#include "PlayerController/DefaultPlayerController.h"
#include "ExpProDev.h"

#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UHeroDefinition* ADefaultGameMode::ResolveSelectedHero() const
{
	if (!HeroRegistry) return nullptr;

	const UExpProGameInstance* GameInstance = UExpProGameInstance::Get(this);
	const FName SelectedHeroId = GameInstance ? GameInstance->GetSelectedHeroId() : NAME_None;

	if (!SelectedHeroId.IsNone())
	{
		if (UHeroDefinition* Selected = HeroRegistry->FindById(SelectedHeroId))
		{
			return Selected;
		}

		UE_LOG(LogExpProDev, Warning,
			TEXT("DefaultGameMode: hero '%s' is not in the registry — falling back to the first entry."),
			*SelectedHeroId.ToString());
	}

	return HeroRegistry->GetDefaultHero();
}

UClass* ADefaultGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UHeroDefinition* Hero = ResolveSelectedHero())
	{
		if (Hero->HeroClass)
		{
			return Hero->HeroClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ADefaultGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
	const FTransform& SpawnTransform)
{
	UHeroDefinition* Hero = ResolveSelectedHero();

	// No registry set, or a definition with no pawn class — behave exactly as before heroes.
	if (!Hero || !Hero->HeroClass)
	{
		return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	}

	// Deferred so the definition is applied before BeginPlay and possession run. AlwaysSpawn
	// matches the existing respawn behaviour — a blocked player start must not eat the pawn.
	AHeroCharacter* Pawn = GetWorld()->SpawnActorDeferred<AHeroCharacter>(
		Hero->HeroClass,
		SpawnTransform,
		nullptr,
		GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Pawn)
	{
		UE_LOG(LogExpProDev, Warning,
			TEXT("DefaultGameMode: failed to spawn hero '%s' — falling back to the default pawn."),
			*Hero->HeroId.ToString());
		return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	}

	Pawn->SetHeroDefinition(Hero);
	UGameplayStatics::FinishSpawningActor(Pawn, SpawnTransform);

	return Pawn;
}

void ADefaultGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bStartRunTimerOnBeginPlay)
	{
		StartRunTimer();
	}
}

void ADefaultGameMode::StartRunTimer()
{
	UWorld* World = GetWorld();
	if (!World) return;

	RunStartTimeSeconds = World->GetTimeSeconds();
	bRunTimerRunning = true;

	const float DurationSeconds = FMath::Max(0.f, RunDurationMinutes * 60.f);

	World->GetTimerManager().ClearTimer(RunExpiryHandle);
	if (DurationSeconds > 0.f)
	{
		World->GetTimerManager().SetTimer(RunExpiryHandle, this,
			&ADefaultGameMode::OnRunTimerFinished, DurationSeconds, false);
	}
	else
	{
		// A zero-length run is already in overtime, and SetTimer with a rate of 0 would just
		// clear the handle rather than fire — so announce the crossing straight away.
		OnRunTimerFinished();
	}

	UE_LOG(LogExpProDev, Log, TEXT("DefaultGameMode: run timer started (%.2f minutes)."), RunDurationMinutes);
}

void ADefaultGameMode::StopRunTimer()
{
	bRunTimerRunning = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RunExpiryHandle);
	}

	UE_LOG(LogExpProDev, Log, TEXT("DefaultGameMode: run timer stopped."));
}

float ADefaultGameMode::GetRunTimeRemaining() const
{
	const UWorld* World = GetWorld();
	if (!bRunTimerRunning || !World) return 0.f;

	const double Elapsed = World->GetTimeSeconds() - RunStartTimeSeconds;
	return static_cast<float>(static_cast<double>(RunDurationMinutes) * 60.0 - Elapsed);
}

bool ADefaultGameMode::IsInOvertime() const
{
	return bRunTimerRunning && GetRunTimeRemaining() < 0.f;
}

void ADefaultGameMode::OnRunTimerFinished()
{
	// The clock deliberately keeps running: GetRunTimeRemaining now goes negative and the overlay
	// counts upwards in red until something calls StopRunTimer.
	UE_LOG(LogExpProDev, Log, TEXT("DefaultGameMode: run timer expired — entering overtime."));

	OnRunTimerExpired.Broadcast();
}

// TECH_DEBT(TD-DEAD-13): VictimController and AttackerController are accepted and then ignored.
// Every caller computes and passes them, so the death path looks like it supports kill attribution,
// scoring and death messages when it does none of that. Either use them or drop the parameters.
void ADefaultGameMode::PlayerEliminated(class APlayerCharacter* EliminatedCharacter,
	class ADefaultPlayerController* VictimController, ADefaultPlayerController* AttackerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminated();
	}
}

void ADefaultGameMode::RequestRespawn(class ACharacter* EliminatedCharacter, class AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if (EliminatedController)
	{
		TArray<AActor*> PlayerStartActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartActors);
		if (PlayerStartActors.Num() > 0)
		{
			const int32 StartSelection = FMath::RandRange(0, PlayerStartActors.Num() - 1);
			RestartPlayerAtPlayerStart(EliminatedController, PlayerStartActors[StartSelection]);
		}
		else
		{
			// No PlayerStarts on this map — fall back to the default spawn transform so we don't crash.
			UE_LOG(LogTemp, Warning, TEXT("RequestRespawn: no PlayerStart actors found; restarting at default transform."));
			RestartPlayer(EliminatedController);
		}
	}
}
