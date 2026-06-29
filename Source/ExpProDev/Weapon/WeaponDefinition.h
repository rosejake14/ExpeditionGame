// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDefinition.generated.h"

class AWeapon;

// One buyable weapon. Mirrors UUpgradeDefinition.
UCLASS(BlueprintType)
class EXPPRODEV_API UWeaponDefinition : public UDataAsset
{
	GENERATED_BODY()
public:
	// Unique save key — never rename this after data has been saved
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName WeaponId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	class UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText Description;

	// Flat DOS$ cost per unit (weapons are consumable, bought in quantity)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 0))
	int32 Cost = 100;

	// Weapon actor spawned in the Extraction Zone for each owned unit (e.g. BP_AssaultRifle)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeapon> WeaponClass;
};
