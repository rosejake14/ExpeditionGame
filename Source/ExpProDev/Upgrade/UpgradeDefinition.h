// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeDefinition.generated.h"

// TECH_DEBT(TD-ARCH-7): upgrade EFFECTS are hardcoded to these two values. Adding a third (move
// speed, stamina, loot magnet) requires C++ changes in three separate places: this enum, a new
// bespoke getter on UUpgradeManagerComponent, and a new apply line in
// APlayerCharacter::LoadPlayerData. A TMap<FName /*Attribute*/, float /*ValuePerRank*/> on the
// definition would make new upgrades pure data authoring.
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	HealthBoost     UMETA(DisplayName = "Health Boost"),
	DamageBoost     UMETA(DisplayName = "Damage Boost"),
};

UCLASS(BlueprintType)
class EXPPRODEV_API UUpgradeDefinition : public UDataAsset
{
	GENERATED_BODY()
public:
	// Unique save key — never rename this after data has been saved
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FName UpgradeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	class UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	EUpgradeType UpgradeType = EUpgradeType::HealthBoost;

	// DOS$ cost for the very first purchase
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade", meta = (ClampMin = 0))
	int32 BaseCost = 50;

	// Cost multiplied by this value per tier already owned
	// e.g. BaseCost=50, Scale=1.5 → tier 1=50, tier 2=75, tier 3=112
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade", meta = (ClampMin = 1.0f))
	float CostScalePerLevel = 1.5f;

	// Bonus added per purchase: raw HP for HealthBoost, fraction (0.1 = 10%) for DamageBoost
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	float ValuePerLevel = 25.f;

	// Maximum purchases allowed (0 = unlimited)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade", meta = (ClampMin = 0))
	int32 MaxPurchases = 5;

	int32 GetCostForNextPurchase(int32 CurrentCount) const
	{
		return FMath::RoundToInt(BaseCost * FMath::Pow(CostScalePerLevel, static_cast<float>(CurrentCount)));
	}

	bool IsMaxed(int32 CurrentCount) const
	{
		return MaxPurchases > 0 && CurrentCount >= MaxPurchases;
	}
};
