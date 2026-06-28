// No Rights Reserved @ Team Expedition

#include "Quest/QuestManagerComponent.h"
#include "Quest/QuestDefinition.h"
#include "Character/PlayerCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "PlayerController/DefaultPlayerController.h"

UQuestManagerComponent::UQuestManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner()))
	{
		if (UInventoryComponent* Inv = Owner->GetInventory())
			Inv->OnInventoryChanged.AddUObject(this, &UQuestManagerComponent::OnInventoryChanged);
	}

	UpdateHUD();
}

void UQuestManagerComponent::AcceptQuest(UQuestDefinition* Quest, AQuestGiverNPC* Giver)
{
	if (!Quest) return;
	ActiveQuest.Definition        = Quest;
	ActiveQuest.QuestGiver        = Giver;
	ActiveQuest.bItemCollected    = false;
	ActiveQuest.bObjectiveComplete = false;
	ActiveQuest.KillCount         = 0;
	UpdateHUD();
	OnInventoryChanged();
}

bool UQuestManagerComponent::TryCompleteQuestFromNPC(AQuestGiverNPC* NPC)
{
	if (!ActiveQuest.Definition || !ActiveQuest.bObjectiveComplete) return false;
	if (ActiveQuest.QuestGiver != NPC) return false;

	NotifyQuestComplete(ActiveQuest.Definition);
	return true;
}

bool UQuestManagerComponent::IsItemCollectedFor(UQuestDefinition* Quest) const
{
	return ActiveQuest.Definition == Quest && ActiveQuest.bItemCollected;
}

void UQuestManagerComponent::NotifyQuestComplete(UQuestDefinition* Quest)
{
	if (!Quest || ActiveQuest.Definition != Quest) return;

	if (APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner()))
	{
		Owner->AddXP(Quest->XPReward);

		// Item Collection only: remove the item on return
		if (Quest->QuestType == EQuestType::ItemCollection)
		{
			if (UInventoryComponent* Inv = Owner->GetInventory())
			{
				for (int32 i = 0; i < Inv->GetTotalSlotCount(); ++i)
				{
					const FInventorySlot& Slot = Inv->GetSlot(i);
					if (Slot.ItemDef == Quest->RequiredItem && Slot.Quantity > 0)
					{
						Inv->RemoveItem(i, Quest->RequiredQuantity);
						break;
					}
				}
			}
		}
	}

	if (ADefaultPlayerController* PC = GetPC())
		PC->SetHUDQuestText(Quest->CompleteText.ToString());

	ActiveQuest = FActiveQuestState();
}

void UQuestManagerComponent::NotifyEnemyKilled(TSubclassOf<ACharacter> KilledClass)
{
	if (!ActiveQuest.Definition) return;
	if (ActiveQuest.Definition->QuestType != EQuestType::EnemyElimination) return;

	// Null EnemyClass on the quest means "any enemy counts"
	if (ActiveQuest.Definition->EnemyClass && !KilledClass->IsChildOf(ActiveQuest.Definition->EnemyClass)) return;

	ActiveQuest.KillCount++;
	UpdateHUD();

	if (ActiveQuest.KillCount >= ActiveQuest.Definition->KillsRequired)
	{
		ActiveQuest.bObjectiveComplete = true;
		UpdateHUD();
	}
}

void UQuestManagerComponent::OnInventoryChanged()
{
	if (!ActiveQuest.Definition || ActiveQuest.bItemCollected) return;
	if (ActiveQuest.Definition->QuestType != EQuestType::ItemCollection) return;

	APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner());
	if (!Owner) return;

	UInventoryComponent* Inv = Owner->GetInventory();
	if (!Inv) return;

	int32 Found = 0;
	for (int32 i = 0; i < Inv->GetTotalSlotCount(); ++i)
	{
		const FInventorySlot& Slot = Inv->GetSlot(i);
		if (Slot.ItemDef == ActiveQuest.Definition->RequiredItem)
			Found += Slot.Quantity;
	}

	if (Found >= ActiveQuest.Definition->RequiredQuantity)
	{
		ActiveQuest.bItemCollected     = true;
		ActiveQuest.bObjectiveComplete = true;
		UpdateHUD();
	}
}

void UQuestManagerComponent::UpdateHUD()
{
	ADefaultPlayerController* PC = GetPC();
	if (!PC) return;

	if (!ActiveQuest.Definition)
	{
		PC->SetHUDQuestText(TEXT(""));
		return;
	}

	if (ActiveQuest.bObjectiveComplete)
	{
		PC->SetHUDQuestText(TEXT("Return to Quest Giver"));
		return;
	}

	FString Text;
	switch (ActiveQuest.Definition->QuestType)
	{
	case EQuestType::ItemCollection:
		Text = ActiveQuest.Definition->CollectObjective.ToString();
		break;

	case EQuestType::EnemyElimination:
		Text = FString::Printf(TEXT("%s: %d/%d"),
			*ActiveQuest.Definition->EliminationObjective.ToString(),
			ActiveQuest.KillCount,
			ActiveQuest.Definition->KillsRequired);
		break;
	}

	PC->SetHUDQuestText(Text);
}

ADefaultPlayerController* UQuestManagerComponent::GetPC()
{
	if (!PlayerController)
	{
		if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
			PlayerController = Cast<ADefaultPlayerController>(Owner->GetController());
	}
	return PlayerController;
}
