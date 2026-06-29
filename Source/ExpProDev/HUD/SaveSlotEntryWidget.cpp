// No Rights Reserved @ Team Expedition

#include "HUD/SaveSlotEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USaveSlotEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (SelectButton)
		SelectButton->OnClicked.AddDynamic(this, &USaveSlotEntryWidget::OnSelectClicked);
	if (DeleteButton)
		DeleteButton->OnClicked.AddDynamic(this, &USaveSlotEntryWidget::OnDeleteClicked);
}

void USaveSlotEntryWidget::InitSlot(int32 Index, bool bExists, int32 Level, int32 Coins)
{
	SlotIndex = Index;

	if (SlotNameText)
		SlotNameText->SetText(FText::FromString(FString::Printf(TEXT("Save %d"), Index + 1)));

	if (SlotInfoText)
	{
		FString Info = bExists
			? FString::Printf(TEXT("Level %d  |  %d DOS$"), Level, Coins)
			: TEXT("Empty");
		SlotInfoText->SetText(FText::FromString(Info));
	}

	// Delete button only makes sense on an occupied slot.
	if (DeleteButton)
		DeleteButton->SetVisibility(bExists ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void USaveSlotEntryWidget::OnSelectClicked()
{
	OnSelected.Broadcast(SlotIndex);
}

void USaveSlotEntryWidget::OnDeleteClicked()
{
	OnDeleteRequested.Broadcast(SlotIndex);
}
