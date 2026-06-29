// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeSlotWidget.generated.h"

class UUpgradeDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeBuyClicked, UUpgradeDefinition*, Def);

UCLASS()
class EXPPRODEV_API UUpgradeSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Broadcast to the parent shop widget when Buy is clicked
	UPROPERTY() FOnUpgradeBuyClicked OnBuyClicked;

	void SetUpgrade(UUpgradeDefinition* Def, int32 CurrentLevel, int32 PlayerCoins);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) class UImage*     UpgradeIcon;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* UpgradeNameText;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* DescriptionText;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* LevelText;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* CostText;
	UPROPERTY(meta = (BindWidget)) class UButton*    BuyButton;

private:
	UPROPERTY() UUpgradeDefinition* CachedDef = nullptr;

	UFUNCTION() void HandleBuyButtonClicked();
};
