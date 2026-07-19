// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopSlotWidget.generated.h"

// One presentable shop row. The parent shop widget builds these from a registry + the economy
// subsystem; the slot widget is a pure view over the struct and knows nothing about saves.
USTRUCT(BlueprintType)
struct FShopEntry
{
	GENERATED_BODY()

	// Save key of the upgrade/weapon this row buys.
	UPROPERTY(BlueprintReadOnly)
	FName Id;

	UPROPERTY(BlueprintReadOnly)
	FText Name;

	UPROPERTY(BlueprintReadOnly)
	FText Description;

	UPROPERTY(BlueprintReadOnly)
	class UTexture2D* Icon = nullptr;

	// e.g. "Level 2 / 5" (upgrades) or "Owned: 3" (weapons) — formatted by the shop widget.
	UPROPERTY(BlueprintReadOnly)
	FText StatusText;

	// e.g. "50 DOS$" or "MAX" — formatted by the shop widget.
	UPROPERTY(BlueprintReadOnly)
	FText CostText;

	// Whether the Buy button should be interactable (affordable and not maxed out).
	UPROPERTY(BlueprintReadOnly)
	bool bBuyEnabled = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopBuyClicked, FName, EntryId);

// Single slot widget shared by both shops. WBP_UpgradeSlot / WBP_WeaponSlot should be reparented
// to this class; its child widgets use the BindWidgetOptional names below.
UCLASS()
class EXPPRODEV_API UShopSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Broadcast to the parent shop widget when Buy is clicked, carrying this row's Id.
	UPROPERTY() FOnShopBuyClicked OnBuyClicked;

	void SetEntry(const FShopEntry& Entry);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional)) class UImage*     IconImage;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* NameText;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* DescriptionText;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* StatusText;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* CostText;
	UPROPERTY(meta = (BindWidgetOptional)) class UButton*    BuyButton;

private:
	FName CachedId;

	UFUNCTION() void HandleBuyButtonClicked();
};
