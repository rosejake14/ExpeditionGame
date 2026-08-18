// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "HUD/ShopWidgetBase.h"
#include "UpgradeShopWidget.generated.h"

class UUpgradeRegistry;

// Thin view over UShopWidgetBase: builds rows from the upgrade registry and routes purchases
// to UEconomySubsystem. All balance/save handling lives in the base + subsystem.
//
// TECH_DEBT(TD-BP-4): this class was reparented onto UShopWidgetBase in C++ (economy refactor).
// WBP_UpgradeShop and WBP_WeaponShop still need re-parenting and re-saving in the editor, and
// their scroll boxes renaming to ListContainer, before either shop renders.
UCLASS()
class EXPPRODEV_API UUpgradeShopWidget : public UShopWidgetBase
{
	GENERATED_BODY()
public:
	// Set to DA_UpgradeRegistry in WBP_UpgradeShop class defaults
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	UUpgradeRegistry* Registry;

protected:
	virtual TArray<FShopEntry> BuildEntries() const override;
	virtual void PurchaseEntry(FName Id) override;
};
