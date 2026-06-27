// No Rights Reserved @ Team Expedition

#include "HUD/QuestSelectionWidget.h"
#include "Quest/QuestDefinition.h"
#include "Quest/QuestManagerComponent.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/PlayerController.h"

void UQuestSelectionWidget::InitQuestList(APlayerCharacter* InPlayer, const TArray<UQuestDefinition*>& Quests)
{
	OwningPlayer = InPlayer;
	QuestOptions.Reset();
	for (UQuestDefinition* Q : Quests)
		if (Q) QuestOptions.Add(Q);

	SetVisibility(ESlateVisibility::Visible);
	OnQuestsReady();

	if (APlayerController* PC = InPlayer ? InPlayer->GetController<APlayerController>() : nullptr)
	{
		PC->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
}

void UQuestSelectionWidget::SelectQuest(UQuestDefinition* Quest)
{
	if (!Quest || !OwningPlayer) return;

	if (UQuestManagerComponent* QM = OwningPlayer->FindComponentByClass<UQuestManagerComponent>())
		QM->AcceptQuest(Quest);

	ClosePanel();
}

void UQuestSelectionWidget::ClosePanel()
{
	SetVisibility(ESlateVisibility::Collapsed);

	if (APlayerController* PC = OwningPlayer ? OwningPlayer->GetController<APlayerController>() : nullptr)
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}
