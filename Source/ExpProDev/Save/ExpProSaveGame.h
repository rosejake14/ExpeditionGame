// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ExpProSaveGame.generated.h"

// TECH_DEBT(TD-ARCH-14): the schema below is everything the game persists. Carried inventory,
// active quests and completed-quest history are all lost on quit, which is why a player can farm
// the same quest repeatedly and why extraction is the only way loot survives a session. Adding
// them needs stable identifiers on UItemDefinition / UQuestDefinition first (TD-ARCH-13), plus a
// CurrentSaveVersion bump and a migration case in USaveGameSubsystem::MigrateIfNeeded.
UCLASS()
class EXPPRODEV_API UExpProSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	// Bumped whenever the persisted schema changes. Drives migration in USaveGameSubsystem::MigrateIfNeeded.
	static constexpr int32 CurrentSaveVersion = 1;

	// Schema version this save was written with. Saves older than CurrentSaveVersion are migrated up on load.
	UPROPERTY()
	int32 SaveVersion = CurrentSaveVersion;

	UPROPERTY(BlueprintReadOnly)
	float XP = 0.f;

	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 DOSCoins = 0;

	UPROPERTY()
	TMap<FName, int32> PurchasedUpgrades;

	// WeaponId -> quantity owned. Consumable: decremented when a weapon is picked up in the world.
	UPROPERTY()
	TMap<FName, int32> PurchasedWeapons;
};
