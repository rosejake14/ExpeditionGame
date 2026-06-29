// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponSlotWidget.generated.h"

class UWeaponDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponBuyClicked, UWeaponDefinition*, Def);

UCLASS()
class EXPPRODEV_API UWeaponSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Broadcast to the parent shop widget when Buy is clicked
	UPROPERTY() FOnWeaponBuyClicked OnBuyClicked;

	void SetWeapon(UWeaponDefinition* Def, int32 OwnedCount, int32 PlayerCoins);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) class UImage*     WeaponIcon;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* WeaponNameText;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* DescriptionText;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* OwnedText;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* CostText;
	UPROPERTY(meta = (BindWidget)) class UButton*    BuyButton;

private:
	UPROPERTY() UWeaponDefinition* CachedDef = nullptr;

	UFUNCTION() void HandleBuyButtonClicked();
};
