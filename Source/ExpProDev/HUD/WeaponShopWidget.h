// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponShopWidget.generated.h"

class UWeaponRegistry;
class UWeaponDefinition;
class UWeaponSlotWidget;
class UExpProSaveGame;

UCLASS()
class EXPPRODEV_API UWeaponShopWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Set to DA_WeaponRegistry in WBP_WeaponShop class defaults
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UWeaponRegistry* Registry;

	// Set to WBP_WeaponSlot in WBP_WeaponShop class defaults
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UWeaponSlotWidget> SlotWidgetClass;

	// Call this from your main menu Blueprint when the screen opens
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void InitShop();

protected:
	UPROPERTY(meta = (BindWidget)) class UScrollBox* WeaponListContainer;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BalanceText;

private:
	UPROPERTY() UExpProSaveGame* CachedSave = nullptr;
	UPROPERTY() TArray<UWeaponSlotWidget*> SlotWidgets;

	void RefreshShop();

	UFUNCTION() void HandleWeaponPurchase(UWeaponDefinition* Def);
};
