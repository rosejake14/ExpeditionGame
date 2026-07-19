// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "HUD/ShopWidgetBase.h"
#include "WeaponShopWidget.generated.h"

class UWeaponRegistry;

// Thin view over UShopWidgetBase: builds rows from the weapon registry and routes purchases
// to UEconomySubsystem. All balance/save handling lives in the base + subsystem.
UCLASS()
class EXPPRODEV_API UWeaponShopWidget : public UShopWidgetBase
{
	GENERATED_BODY()
public:
	// Set to DA_WeaponRegistry in WBP_WeaponShop class defaults
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UWeaponRegistry* Registry;

protected:
	virtual TArray<FShopEntry> BuildEntries() const override;
	virtual void PurchaseEntry(FName Id) override;
};
