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
	SpawnEnemies();
}

void AEnemySpawner::SpawnEnemies()
{
	if (SpawnEntries.IsEmpty()) return;

	TArray<int32> SpawnedCounts;
	SpawnedCounts.SetNum(SpawnEntries.Num());

	int32 TotalSpawned = 0;
	const int32 MaxAttempts = MaxTotalEnemies * 3; // guard against repeated spawn failures
	int32 Attempts = 0;

	while (TotalSpawned < MaxTotalEnemies && Attempts < MaxAttempts)
	{
		++Attempts;

		// Build weighted pool from entries that still have quota remaining
		float TotalWeight = 0.f;
		for (int32 i = 0; i < SpawnEntries.Num(); ++i)
		{
			const FEnemySpawnEntry& Entry = SpawnEntries[i];
			if (Entry.EnemyClass && SpawnedCounts[i] < Entry.MaxCount)
			{
				TotalWeight += Entry.SpawnWeight;
			}
		}

		if (TotalWeight <= 0.f) break; // all entries exhausted

		// Weighted random pick
		float Roll = FMath::FRandRange(0.f, TotalWeight);
		float Accumulated = 0.f;
		int32 ChosenIndex = -1;
		for (int32 i = 0; i < SpawnEntries.Num(); ++i)
		{
			const FEnemySpawnEntry& Entry = SpawnEntries[i];
			if (!Entry.EnemyClass || SpawnedCounts[i] >= Entry.MaxCount) continue;

			Accumulated += Entry.SpawnWeight;
			if (Roll <= Accumulated)
			{
				ChosenIndex = i;
				break;
			}
		}

		if (ChosenIndex < 0) continue;

		// Random point inside the spawn radius circle
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Distance = FMath::FRandRange(0.f, SpawnRadius);
		const FVector Offset(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);
		const FVector CandidateXY = GetActorLocation() + Offset;

		// Trace downward from well above to find the actual ground surface
		const FVector TraceStart(CandidateXY.X, CandidateXY.Y, CandidateXY.Z + 2000.f);
		const FVector TraceEnd  (CandidateXY.X, CandidateXY.Y, CandidateXY.Z - 2000.f);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		FVector SpawnLocation = CandidateXY;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			SpawnLocation = HitResult.ImpactPoint;

			// Lift by the capsule half-height so the character stands on the surface
			const ACharacter* CDO = SpawnEntries[ChosenIndex].EnemyClass->GetDefaultObject<ACharacter>();
			if (CDO && CDO->GetCapsuleComponent())
			{
				SpawnLocation.Z += CDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			}
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ACharacter* Spawned = GetWorld()->SpawnActor<ACharacter>(
			SpawnEntries[ChosenIndex].EnemyClass, SpawnLocation, FRotator::ZeroRotator, Params);

		if (Spawned)
		{
			SpawnedCounts[ChosenIndex]++;
			TotalSpawned++;
		}
	}
}
