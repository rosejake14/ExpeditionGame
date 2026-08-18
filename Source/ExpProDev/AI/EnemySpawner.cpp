// No Rights Reserved @ Team Expedition

#include "AI/EnemySpawner.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	RangeVisualization = CreateDefaultSubobject<USphereComponent>(TEXT("RangeVisualization"));
	RootComponent = RangeVisualization;
	RangeVisualization->SetSphereRadius(SpawnRadius);
	RangeVisualization->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RangeVisualization->SetHiddenInGame(true);

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(RootComponent);
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinder<UTexture2D> Texture;
			FConstructorStatics() : Texture(TEXT("/Engine/EditorResources/Spawn_Point")) {}
		};
		static FConstructorStatics ConstructorStatics;
		if (ConstructorStatics.Texture.Succeeded())
			SpriteComponent->SetSprite(ConstructorStatics.Texture.Object);
	}
#endif
}

void AEnemySpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RangeVisualization->SetSphereRadius(SpawnRadius);
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnEntries.IsEmpty() || MaxTotalEnemies <= 0) return;

	SpawnedCounts.SetNum(SpawnEntries.Num());
	TotalSpawned = 0;

	SpawnBatch();
	if (TotalSpawned < MaxTotalEnemies || bReplenishPhase)
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnBatch, SpawnInterval, true);
}

void AEnemySpawner::SpawnBatch()
{
	// ---- Replenish phase (continuous spawners only) ----
	if (bReplenishPhase)
	{
		const int32 Alive = CountAliveEnemies();
		if (Alive >= ReplenishThreshold) return;

		const int32 Needed = ReplenishTarget - Alive;
		for (int32 i = 0; i < Needed; ++i)
			TrySpawnOne(true);

		return;
	}

	// ---- Phase 1: initial burst ----
	int32 SpawnedThisBatch = 0;
	const int32 MaxAttempts = SpawnBatchSize * 3;
	int32 Attempts = 0;

	while (SpawnedThisBatch < SpawnBatchSize && TotalSpawned < MaxTotalEnemies && Attempts < MaxAttempts)
	{
		++Attempts;

		// Check whether any entry still has quota — if not, no point retrying
		bool bAnyEligible = false;
		for (int32 i = 0; i < SpawnEntries.Num(); ++i)
		{
			if (SpawnEntries[i].EnemyClass && SpawnedCounts[i] < SpawnEntries[i].MaxCount)
			{
				bAnyEligible = true;
				break;
			}
		}

		if (!bAnyEligible)
		{
			// All per-type quotas exhausted before MaxTotalEnemies was reached
			if (bContinuousSpawning)
				bReplenishPhase = true;
			else
				GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
			return;
		}

		if (TrySpawnOne(false))
			++SpawnedThisBatch;
	}

	if (TotalSpawned >= MaxTotalEnemies)
	{
		if (bContinuousSpawning)
			bReplenishPhase = true;
		else
			GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

int32 AEnemySpawner::CountAliveEnemies()
{
	AliveEnemies.RemoveAll([](const TWeakObjectPtr<ACharacter>& E) { return !E.IsValid(); });
	return AliveEnemies.Num();
}

bool AEnemySpawner::TrySpawnOne(bool bIgnoreTypeCaps)
{
	// TECH_DEBT(TD-ARCH-21): the eligibility filter is written out twice below (once to total the
	// weights, once to pick) and has to be kept in sync by hand.
	// Build weighted pool
	float TotalWeight = 0.f;
	for (int32 i = 0; i < SpawnEntries.Num(); ++i)
	{
		if (!SpawnEntries[i].EnemyClass) continue;
		if (!bIgnoreTypeCaps && SpawnedCounts[i] >= SpawnEntries[i].MaxCount) continue;
		TotalWeight += SpawnEntries[i].SpawnWeight;
	}
	if (TotalWeight <= 0.f) return false;

	// Weighted random pick
	float Roll = FMath::FRandRange(0.f, TotalWeight);
	float Accumulated = 0.f;
	int32 ChosenIndex = -1;
	for (int32 i = 0; i < SpawnEntries.Num(); ++i)
	{
		if (!SpawnEntries[i].EnemyClass) continue;
		if (!bIgnoreTypeCaps && SpawnedCounts[i] >= SpawnEntries[i].MaxCount) continue;
		Accumulated += SpawnEntries[i].SpawnWeight;
		if (Roll <= Accumulated) { ChosenIndex = i; break; }
	}
	if (ChosenIndex < 0) return false;

	// Random point inside spawn radius, traced to ground
	// TECH_DEBT(TD-BUG-18): the candidate point is only traced to geometry — it is never projected
	// onto the navmesh. Enemies spawned off-nav can never path, so they stand still forever; the
	// spawner still counts them as alive and stops replenishing. Use ProjectPointToNavigation.
	// TECH_DEBT(TD-ARCH-20): the ±2000uu trace span is a magic number repeated in
	// AItemPickup::GroundedLocation — one shared ground-snap helper should own it.
	const float Angle    = FMath::FRandRange(0.f, 2.f * PI);
	const float Distance = FMath::FRandRange(0.f, SpawnRadius);
	const FVector CandidateXY = GetActorLocation()
		+ FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FVector SpawnLocation = CandidateXY;
	if (GetWorld()->LineTraceSingleByChannel(HitResult,
		FVector(CandidateXY.X, CandidateXY.Y, CandidateXY.Z + 2000.f),
		FVector(CandidateXY.X, CandidateXY.Y, CandidateXY.Z - 2000.f),
		ECC_WorldStatic, QueryParams))
	{
		SpawnLocation = HitResult.ImpactPoint;
		const ACharacter* CDO = SpawnEntries[ChosenIndex].EnemyClass->GetDefaultObject<ACharacter>();
		if (CDO && CDO->GetCapsuleComponent())
			SpawnLocation.Z += CDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACharacter* Spawned = GetWorld()->SpawnActor<ACharacter>(
		SpawnEntries[ChosenIndex].EnemyClass, SpawnLocation, FRotator::ZeroRotator, Params);

	if (!Spawned) return false;

	AliveEnemies.Add(Spawned);
	TotalSpawned++;
	if (!bIgnoreTypeCaps)
		SpawnedCounts[ChosenIndex]++;

	return true;
}
