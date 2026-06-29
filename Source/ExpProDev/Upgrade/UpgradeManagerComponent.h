// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UpgradeManagerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EXPPRODEV_API UUpgradeManagerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	// Set to DA_UpgradeRegistry on BP_PlayerCharacter's component instance
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade")
	class UUpgradeRegistry* Registry;

	// Called from PlayerCharacter::LoadPlayerData — loads purchased counts from save
	void LoadUpgrades(const TMap<FName, int32>& Saved);

	// How many times a given upgrade has been purchased
	int32 GetPurchaseCount(FName UpgradeId) const;

	// Full map — used by SavePlayerData to persist upgrades back to disk
	const TMap<FName, int32>& GetAllPurchases() const { return PurchasedUpgrades; }

	// Resets all purchases — called by WipeSave
	void ClearUpgrades() { PurchasedUpgrades.Empty(); }

	// Summed bonus values — PlayerCharacter reads these and applies them to its stats
	float GetHealthBonus() const;
	float GetDamageMultiplierBonus() const; // additive: 0.1 = +10% damage

private:
	TMap<FName, int32> PurchasedUpgrades;
};
