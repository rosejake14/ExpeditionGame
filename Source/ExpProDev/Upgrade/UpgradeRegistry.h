// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeRegistry.generated.h"

// Single asset containing all available upgrades.
// Both the shop widget and UpgradeManagerComponent reference this —
// adding a new upgrade DA here is all a designer needs to do.
UCLASS(BlueprintType)
class EXPPRODEV_API UUpgradeRegistry : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TArray<class UUpgradeDefinition*> Upgrades;

	// Convenience: find a definition by its save key (mirrors UWeaponRegistry::FindById).
	class UUpgradeDefinition* FindById(FName UpgradeId) const;
};
