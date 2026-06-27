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
		{
			SpriteComponent->SetSprite(ConstructorStatics.Texture.Object);
		}
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

	// First batch fires immediately, then repeats every SpawnInterval
	SpawnBatch();
	if (TotalSpawned < MaxTotalEnemies)
	{
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnBatch, SpawnInterval, true);
	}
}

void AEnemySpawner::SpawnBatch()
{
	int32 SpawnedThisBatch = 0;
	const int32 MaxAttemptsPerBatch = SpawnBatchSize * 3;
	int32 Attempts = 0;

	while (SpawnedThisBatch < SpawnBatchSize && TotalSpawned < MaxTotalEnemies && Attempts < MaxAttemptsPerBatch)
	{
		++Attempts;

		// Build weighted pool from entries that still have quota remaining
		float TotalWeight = 0.f;
		for (int32 i = 0; i < SpawnEntries.Num(); ++i)
		{
			if (SpawnEntries[i].EnemyClass && SpawnedCounts[i] < SpawnEntries[i].MaxCount)
				TotalWeight += SpawnEntries[i].SpawnWeight;
		}

		if (TotalWeight <= 0.f)
		{
			GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
			return;
		}

		// Weighted random pick
		float Roll = FMath::FRandRange(0.f, TotalWeight);
		float Accumulated = 0.f;
		int32 ChosenIndex = -1;
		for (int32 i = 0; i < SpawnEntries.Num(); ++i)
		{
			if (!SpawnEntries[i].EnemyClass || SpawnedCounts[i] >= SpawnEntries[i].MaxCount) continue;
			Accumulated += SpawnEntries[i].SpawnWeight;
			if (Roll <= Accumulated) { ChosenIndex = i; break; }
		}

		if (ChosenIndex < 0) continue;

		// Random point inside the spawn radius, traced to ground
		const float Angle    = FMath::FRandRange(0.f, 2.f * PI);
		const float Distance = FMath::FRandRange(0.f, SpawnRadius);
		const FVector CandidateXY = GetActorLocation() + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);

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

		if (GetWorld()->SpawnActor<ACharacter>(SpawnEntries[ChosenIndex].EnemyClass, SpawnLocation, FRotator::ZeroRotator, Params))
		{
			SpawnedCounts[ChosenIndex]++;
			TotalSpawned++;
			SpawnedThisBatch++;
		}
	}

	if (TotalSpawned >= MaxTotalEnemies)
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}
