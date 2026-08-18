// No Rights Reserved @ Team Expedition

#include "HUD/SaveSlotSelectWidget.h"
#include "HUD/SaveSlotEntryWidget.h"
#include "Save/ExpProSaveGame.h"
#include "Save/SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"

void USaveSlotSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// TECH_DEBT(TD-BUG-17): the three BindWidget pointers are dereferenced with no null guard. If
	// the widget BP is missing (or renames) SlotEntry_0/1/2 this crashes on construct instead of
	// logging. The slot count is also hardcoded to three here and in RefreshSlots.
	SlotEntry_0->OnSelected.AddDynamic(this, &USaveSlotSelectWidget::HandleSlotSelected);
	SlotEntry_1->OnSelected.AddDynamic(this, &USaveSlotSelectWidget::HandleSlotSelected);
	SlotEntry_2->OnSelected.AddDynamic(this, &USaveSlotSelectWidget::HandleSlotSelected);

	SlotEntry_0->OnDeleteRequested.AddDynamic(this, &USaveSlotSelectWidget::HandleSlotDeleted);
	SlotEntry_1->OnDeleteRequested.AddDynamic(this, &USaveSlotSelectWidget::HandleSlotDeleted);
	SlotEntry_2->OnDeleteRequested.AddDynamic(this, &USaveSlotSelectWidget::HandleSlotDeleted);

	RefreshSlots();
}

void USaveSlotSelectWidget::RefreshSlots()
{
	TArray<USaveSlotEntryWidget*> Entries = { SlotEntry_0, SlotEntry_1, SlotEntry_2 };
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		const FString SlotName = USaveGameSubsystem::GetSlotName(i);
		const bool bExists = UGameplayStatics::DoesSaveGameExist(SlotName, 0);

		int32 Level = 1;
		int32 Coins = 0;
		if (bExists)
		{
			if (UExpProSaveGame* Save = Cast<UExpProSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
			{
				Level = Save->Level;
				Coins = Save->DOSCoins;
			}
		}

		Entries[i]->InitSlot(i, bExists, Level, Coins);
	}
}

void USaveSlotSelectWidget::HandleSlotSelected(int32 SlotIndex)
{
	USaveGameSubsystem* Sub = GetWorld()->GetGameInstance()->GetSubsystem<USaveGameSubsystem>();
	if (!Sub) return;

	const FString SlotName = USaveGameSubsystem::GetSlotName(SlotIndex);
	const bool bIsNewSave = !UGameplayStatics::DoesSaveGameExist(SlotName, 0);

	// Make this the active slot first so the save API resolves writes to it.
	Sub->ActiveSlotIndex = SlotIndex;

	if (bIsNewSave)
	{
		// Write a blank (current-version) save to disk immediately so the slot shows as occupied on return.
		USaveGameSubsystem::MutateActiveSlot(this, [](UExpProSaveGame&){});
	}

	OnSlotSelected(SlotIndex, bIsNewSave);
}

void USaveSlotSelectWidget::HandleSlotDeleted(int32 SlotIndex)
{
	UGameplayStatics::DeleteGameInSlot(USaveGameSubsystem::GetSlotName(SlotIndex), 0);

	// If the deleted slot is the cached active one, drop the cache so a queued flush can't recreate it.
	if (USaveGameSubsystem* Sub = GetWorld()->GetGameInstance()->GetSubsystem<USaveGameSubsystem>())
	{
		if (Sub->ActiveSlotIndex == SlotIndex)
			USaveGameSubsystem::DiscardCache(this);
	}

	RefreshSlots();
}
