// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveSlotEntryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveSlotSelected, int32, SlotIndex);

/**
 * One save-file card. Create three of these inside WBP_SaveSlotSelect,
 * named SlotEntry_0 / SlotEntry_1 / SlotEntry_2.
 *
 * Required named widgets in the Blueprint:
 *   SelectButton  (Button)
 *   SlotNameText  (TextBlock)  — shows "Save 1" / "Save 2" / "Save 3"
 *   SlotInfoText  (TextBlock)  — shows "Level X | Y DOS$" or "Empty"
 */
UCLASS()
class EXPPRODEV_API USaveSlotEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Parent widget binds to this to know which slot was clicked.
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnSaveSlotSelected OnSelected;

	// Parent widget binds to this to handle deletion.
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnSaveSlotSelected OnDeleteRequested;

	// Called by USaveSlotSelectWidget to populate the card before display.
	UFUNCTION(BlueprintCallable, Category = "Save")
	void InitSlot(int32 Index, bool bExists, int32 Level, int32 Coins);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) class UButton*    SelectButton;
	UPROPERTY(meta = (BindWidget)) class UButton*    DeleteButton;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* SlotNameText;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* SlotInfoText;

private:
	int32 SlotIndex = 0;

	UFUNCTION() void OnSelectClicked();
	UFUNCTION() void OnDeleteClicked();
};
