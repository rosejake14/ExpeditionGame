// No Rights Reserved @ Team Expedition

#include "HUD/InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Inventory/ItemDefinition.h"

void UInventorySlotWidget::UpdateSlot(const FInventorySlot& InSlot, bool bIsActive)
{
	if (InSlot.IsEmpty())
	{
		if (ItemIcon) ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (QuantityText) QuantityText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		if (ItemIcon)
		{
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
			if (InSlot.ItemDef->Icon)
				ItemIcon->SetBrushFromTexture(InSlot.ItemDef->Icon, true);
		}
		if (QuantityText)
		{
			bool bShowQty = InSlot.ItemDef->MaxStackSize > 1 && InSlot.Quantity > 1;
			QuantityText->SetVisibility(bShowQty ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			if (bShowQty)
				QuantityText->SetText(FText::AsNumber(InSlot.Quantity));
		}
	}

	if (ActiveBorder)
		ActiveBorder->SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
