// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryScreenWidget.generated.h"

UCLASS()
class EXPPRODEV_API UInventoryScreenWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	// Called once both HUD and pawn exist — same dual-path pattern as HotbarWidget
	void InitInventory(class UInventoryComponent* Inventory);

	// Flips between Visible and Collapsed; forces a refresh on open
	void ToggleScreen();

	// Force-refresh all slot visuals
	void Refresh();

	// Optional bind — if WBP_InventoryScreen has a UniformGridPanel named SlotGrid it is
	// used automatically; otherwise NativeConstruct creates one so no Blueprint setup is required.
	UPROPERTY(meta = (BindWidgetOptional))
	class UUniformGridPanel* SlotGrid;

	// Set this in the Blueprint to WBP_InventorySlot; if left null a plain slot widget is used
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<class UInventorySlotWidget> SlotWidgetClass;

private:
	UPROPERTY()
	class UInventoryComponent* InventoryComp;

	UPROPERTY()
	TArray<class UInventorySlotWidget*> SlotWidgets;
};
