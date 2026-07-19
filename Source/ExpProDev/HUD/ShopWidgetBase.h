// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD/ShopSlotWidget.h"
#include "ShopWidgetBase.generated.h"

class UEconomySubsystem;

/**
 * Shared plumbing for the DOS$ shops: builds a row per entry, routes Buy clicks to the economy
 * subsystem, and refreshes purely from the subsystem's OnBalanceChanged / OnPurchaseCompleted
 * delegates. Subclasses only supply BuildEntries() and PurchaseEntry().
 */
UCLASS(Abstract)
class EXPPRODEV_API UShopWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	// Set to the reparented WBP_ShopSlot in the shop's class defaults.
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UShopSlotWidget> SlotWidgetClass;

	// Rebuilds every row from scratch. Called on NativeConstruct; also callable from BP.
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitShop();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Rename the shop's list/balance children to these names in the widget BP.
	UPROPERTY(meta = (BindWidgetOptional)) class UScrollBox* ListContainer;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* BalanceText;

	// Subclasses implement these two — everything else is shared.
	virtual TArray<FShopEntry> BuildEntries() const { return TArray<FShopEntry>(); }
	virtual void PurchaseEntry(FName Id) {}

	UEconomySubsystem* GetEconomy() const;

	// Refresh row contents + balance text without rebuilding the widget tree.
	void RefreshShop();

private:
	UPROPERTY() TArray<UShopSlotWidget*> SlotWidgets;

	void SetBalanceText(int32 Balance);

	UFUNCTION() void HandleBuyClicked(FName Id);
	UFUNCTION() void HandleBalanceChanged(int32 NewBalance);
	UFUNCTION() void HandlePurchaseCompleted(FName Id);
};
