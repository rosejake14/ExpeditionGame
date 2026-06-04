// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
#include "LootTable.generated.h"

USTRUCT(BlueprintType)
struct FLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	UItemDefinition* Item = nullptr;

	// The pickup actor to spawn — set this to a BP_ItemPickup with the right mesh for this item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TSubclassOf<class AItemPickup> PickupActorClass;

	// 0 = never drops, 1 = always drops
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float DropChance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 1))
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 1))
	int32 MaxQuantity = 1;
};

USTRUCT(BlueprintType)
struct FLootTable
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FLootEntry> Entries;
};
