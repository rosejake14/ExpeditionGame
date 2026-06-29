// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponRegistry.generated.h"

// Single asset listing all buyable weapons. Referenced by both the weapon shop
// widget and the Extraction Zone — adding a weapon DA here is all a designer needs to do.
UCLASS(BlueprintType)
class EXPPRODEV_API UWeaponRegistry : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<class UWeaponDefinition*> Weapons;

	// Convenience: find a definition by its save key.
	UWeaponDefinition* FindById(FName WeaponId) const;
};
