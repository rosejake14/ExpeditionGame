// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
	GENERATED_BODY()

	// Enemy class to spawn
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<ACharacter> EnemyClass;

	// Max number of this type that can be spawned during the initial burst
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = 0))
	int32 MaxCount = 3;

	// Relative likelihood this type is chosen. Higher = picked more often.
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = 0.f))
	float SpawnWeight = 1.f;
};

UCLASS()
class EXPPRODEV_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
public:
	AEnemySpawner();

	// Enemy types and per-type limits
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TArray<FEnemySpawnEntry> SpawnEntries;

	// Total enemies to spawn in the initial burst before the spawner switches behaviour
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = 0))
	int32 MaxTotalEnemies = 10;

	// How many enemies to spawn per batch
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = 1))
	int32 SpawnBatchSize = 1;

	// Seconds between each batch
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = 0.f))
	float SpawnInterval = 5.f;

	// Radius around this actor in which enemies are placed
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = 0.f))
	float SpawnRadius = 1000.f;

	// When true, after the initial burst this spawner keeps the world topped up with enemies.
	UPROPERTY(EditAnywhere, Category = "Spawner|Continuous")
	bool bContinuousSpawning = false;

	// If alive enemies from this spawner drops below this number, start replenishing.
	UPROPERTY(EditAnywhere, Category = "Spawner|Continuous",
		meta = (ClampMin = 0, EditCondition = "bContinuousSpawning"))
	int32 ReplenishThreshold = 3;

	// Keep spawning until alive count reaches this number.
	UPROPERTY(EditAnywhere, Category = "Spawner|Continuous",
		meta = (ClampMin = 0, EditCondition = "bContinuousSpawning"))
	int32 ReplenishTarget = 5;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* RangeVisualization;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	class UBillboardComponent* SpriteComponent;
#endif

	TArray<int32> SpawnedCounts;
	int32 TotalSpawned = 0;
	bool bReplenishPhase = false;

	// Weak pointers to every enemy spawned — used to count how many are still alive.
	TArray<TWeakObjectPtr<ACharacter>> AliveEnemies;

	FTimerHandle SpawnTimerHandle;

	void SpawnBatch();

	// Purges dead entries and returns the current alive count.
	int32 CountAliveEnemies();

	// Attempts to pick a weighted entry and spawn one enemy.
	// bIgnoreTypeCaps: when true, per-type MaxCount is not checked (used in replenish phase).
	// Returns true if an enemy was successfully spawned.
	bool TrySpawnOne(bool bIgnoreTypeCaps);
};
