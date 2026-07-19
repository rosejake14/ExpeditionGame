// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Extraction/ExtractionTypes.h"
#include "EconomySubsystem.generated.h"

class UUpgradeRegistry;
class UWeaponRegistry;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBalanceChanged, int32, NewBalance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPurchaseCompleted, FName, Id);

/**
 * Single owner of DOS$ transactions: shop purchases and extraction selling.
 * Widgets and the pawn call through here instead of touching the save directly, so all
 * economy writes funnel through USaveGameSubsystem::MutateActiveSlot and stay consistent.
 */
UCLASS()
class EXPPRODEV_API UEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// Fires after any balance change (purchase or sell). UI binds to this instead of polling the save.
	UPROPERTY(BlueprintAssignable, Category = "Economy")
	FOnBalanceChanged OnBalanceChanged;

	// Fires after a successful purchase, carrying the purchased item's Id.
	UPROPERTY(BlueprintAssignable, Category = "Economy")
	FOnPurchaseCompleted OnPurchaseCompleted;

	// Current DOS$ in the active save slot (reads the cached save when one exists).
	UFUNCTION(BlueprintCallable, Category = "Economy")
	int32 GetBalance() const;

	// How many times an upgrade / how many of a weapon are owned in the active slot.
	int32 GetUpgradeCount(FName UpgradeId) const;
	int32 GetWeaponOwnedCount(FName WeaponId) const;

	// Validate cost against the active balance, deduct, bump the purchase map, persist. Returns
	// true only if the purchase actually happened; broadcasts OnBalanceChanged/OnPurchaseCompleted.
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool TryPurchaseUpgrade(FName UpgradeId, const UUpgradeRegistry* Registry);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool TryPurchaseWeapon(FName WeaponId, const UWeaponRegistry* Registry);

	// Values every Loot-type slot in Inventory, removes those slots, credits the total, and
	// returns the entries + totals for the sell summary UI. Broadcasts OnBalanceChanged.
	FSellResult SellLoot(UInventoryComponent* Inventory);

	// Resolve the subsystem from any world context (nullptr if unavailable).
	static UEconomySubsystem* Get(const UObject* WorldContext);
};
