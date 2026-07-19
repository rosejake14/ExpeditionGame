// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "HUD/ShopWidgetBase.h"
#include "UpgradeShopWidget.generated.h"

class UUpgradeRegistry;

// Thin view over UShopWidgetBase: builds rows from the upgrade registry and routes purchases
// to UEconomySubsystem. All balance/save handling lives in the base + subsystem.
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
