// No Rights Reserved @ Team Expedition

#include "HUD/HotbarWidget.h"
#include "Components/HorizontalBox.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "HUD/InventorySlotWidget.h"

void UHotbarWidget::InitHotbar(UInventoryComponent* Inventory)
{
	InventoryComp = Inventory;
	if (!InventoryComp) { UE_LOG(LogTemp, Error, TEXT("HotbarWidget: InventoryComp is null")); return; }
	if (!SlotContainer) { UE_LOG(LogTemp, Error, TEXT("HotbarWidget: SlotContainer is null — check WBP_Hotbar has a HorizontalBox named SlotContainer")); return; }
	if (!SlotWidgetClass) { UE_LOG(LogTemp, Error, TEXT("HotbarWidget: SlotWidgetClass is null — set SlotWidgetClass to WBP_InventorySlot in WBP_Hotbar class defaults")); return; }

	SlotContainer->ClearChildren();
	SlotWidgets.Empty();

	for (int32 i = 0; i < InventoryComp->GetHotbarSlotCount(); i++)
	{
		UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
		if (SlotWidget)
		{
			SlotContainer->AddChildToHorizontalBox(SlotWidget);
			SlotWidgets.Add(SlotWidget);
		}
	}

	InventoryComp->OnInventoryChanged.AddUObject(this, &UHotbarWidget::Refresh);
	Refresh();
}

void UHotbarWidget::Refresh()
{
	if (!InventoryComp) return;
	for (int32 i = 0; i < SlotWidgets.Num(); i++)
	{
		bool bIsActive = (i == InventoryComp->ActiveHotbarIndex);
		SlotWidgets[i]->UpdateSlot(InventoryComp->GetSlot(i), bIsActive);
	}
}
