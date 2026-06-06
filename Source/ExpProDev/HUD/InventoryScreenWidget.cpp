// No Rights Reserved @ Team Expedition

#include "HUD/InventoryScreenWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Inventory/InventoryComponent.h"
#include "HUD/InventorySlotWidget.h"

void UInventoryScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// BindWidgetOptional: SlotGrid is already set if WBP named it correctly.
	// If not, build the grid here so no Blueprint setup is required.
	if (!SlotGrid)
	{
		SlotGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("SlotGrid"));

		// Try to add to the Blueprint's root panel; if no root exists, make the grid the root.
		if (UPanelWidget* Root = Cast<UPanelWidget>(WidgetTree->RootWidget))
		{
			Root->AddChild(SlotGrid);
		}
		else
		{
			WidgetTree->RootWidget = SlotGrid;
		}
	}
}

void UInventoryScreenWidget::InitInventory(UInventoryComponent* Inventory)
{
	InventoryComp = Inventory;
	if (!InventoryComp) { UE_LOG(LogTemp, Error, TEXT("InventoryScreenWidget: InventoryComp is null")); return; }
	if (!SlotGrid)      { UE_LOG(LogTemp, Error, TEXT("InventoryScreenWidget: SlotGrid still null after NativeConstruct")); return; }
	if (!SlotWidgetClass) { UE_LOG(LogTemp, Error, TEXT("InventoryScreenWidget: SlotWidgetClass not set — assign WBP_InventorySlot in class defaults")); return; }

	SlotGrid->ClearChildren();
	SlotWidgets.Empty();

	const int32 TotalSlots = InventoryComp->GetTotalSlotCount();
	const int32 Columns    = InventoryComp->GetHotbarSlotCount();

	for (int32 i = 0; i < TotalSlots; i++)
	{
		UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
		if (SlotWidget)
		{
			SlotGrid->AddChildToUniformGrid(SlotWidget, i / Columns, i % Columns);
			SlotWidgets.Add(SlotWidget);
		}
	}

	// RemoveAll guards against double-binding when InitInventory is called via both code paths
	InventoryComp->OnInventoryChanged.RemoveAll(this);
	InventoryComp->OnInventoryChanged.AddUObject(this, &UInventoryScreenWidget::Refresh);
	SetVisibility(ESlateVisibility::Collapsed);
	Refresh();
}

void UInventoryScreenWidget::ToggleScreen()
{
	const bool bIsOpen = GetVisibility() == ESlateVisibility::Visible;
	SetVisibility(bIsOpen ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (!bIsOpen)
	{
		Refresh();
	}
}

void UInventoryScreenWidget::Refresh()
{
	if (!InventoryComp) return;
	const int32 HotbarCount = InventoryComp->GetHotbarSlotCount();
	for (int32 i = 0; i < SlotWidgets.Num(); i++)
	{
		const bool bIsActive = (i == InventoryComp->ActiveHotbarIndex && i < HotbarCount);
		SlotWidgets[i]->UpdateSlot(InventoryComp->GetSlot(i), bIsActive);
	}
}
