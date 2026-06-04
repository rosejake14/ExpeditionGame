// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryComponent.h"
#include "InventorySlotWidget.generated.h"

UCLASS()
class EXPPRODEV_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateSlot(const FInventorySlot& InSlot, bool bIsActive);

	UPROPERTY(meta = (BindWidget))
	class UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* QuantityText;

	// Optional — highlight border shown on the active hotbar slot
	UPROPERTY(meta = (BindWidgetOptional))
	class UBorder* ActiveBorder;
};
