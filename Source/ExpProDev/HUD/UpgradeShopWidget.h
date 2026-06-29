// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeShopWidget.generated.h"

class UUpgradeRegistry;
class UUpgradeDefinition;
class UUpgradeSlotWidget;
class UExpProSaveGame;

UCLASS()
class EXPPRODEV_API UUpgradeShopWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Set to DA_UpgradeRegistry in WBP_UpgradeShop class defaults
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	UUpgradeRegistry* Registry;

	// Set to WBP_UpgradeSlot in WBP_UpgradeShop class defaults
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	TSubclassOf<UUpgradeSlotWidget> SlotWidgetClass;

	// Call this from your main menu Blueprint on BeginPlay (or when the screen opens)
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void InitShop();

protected:
	UPROPERTY(meta = (BindWidget)) class UScrollBox* UpgradeListContainer;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BalanceText;

private:
	UPROPERTY() UExpProSaveGame* CachedSave = nullptr;
	UPROPERTY() TArray<UUpgradeSlotWidget*> SlotWidgets;

	void RefreshShop();

	UFUNCTION() void HandleUpgradePurchase(UUpgradeDefinition* Def);
};
