// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveSlotSelectWidget.generated.h"

class USaveSlotEntryWidget;

/**
 * The save-file selection screen. Set the parent class of WBP_SaveSlotSelect to this.
 *
 * The Blueprint must contain three USaveSlotEntryWidget children with these exact names:
 *   SlotEntry_0 / SlotEntry_1 / SlotEntry_2
 *
 * Implement OnSlotSelected in the Blueprint to open the game map (or do whatever comes next).
 */
UCLASS()
class EXPPRODEV_API USaveSlotSelectWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) USaveSlotEntryWidget* SlotEntry_0;
	UPROPERTY(meta = (BindWidget)) USaveSlotEntryWidget* SlotEntry_1;
	UPROPERTY(meta = (BindWidget)) USaveSlotEntryWidget* SlotEntry_2;

	// Implement this in Blueprint — open the map, dismiss the widget, etc.
	// bIsNewSave is true when the player chose an empty slot (so you know to skip a "continue?" prompt).
	// TECH_DEBT(TD-BP-1): no C++ fallback — if the BP doesn't implement this, picking a save slot
	// marks it active and then does nothing, stranding the player on the menu.
	UFUNCTION(BlueprintImplementableEvent, Category = "Save")
	void OnSlotSelected(int32 SlotIndex, bool bIsNewSave);

private:
	void RefreshSlots();

	UFUNCTION() void HandleSlotSelected(int32 SlotIndex);
	UFUNCTION() void HandleSlotDeleted(int32 SlotIndex);
};
