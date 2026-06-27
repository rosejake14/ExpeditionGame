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

void UQuestManagerComponent::AcceptQuest(UQuestDefinition* Quest)
{
	if (!Quest) return;
	ActiveQuest.Definition     = Quest;
	ActiveQuest.bItemCollected = false;
	UpdateHUD();
}

bool UQuestManagerComponent::IsItemCollectedFor(UQuestDefinition* Quest) const
{
	return ActiveQuest.Definition == Quest && ActiveQuest.bItemCollected;
}

void UQuestManagerComponent::NotifyQuestComplete(UQuestDefinition* Quest)
{
	if (!Quest || ActiveQuest.Definition != Quest) return;

	// Grant XP
	if (APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner()))
	{
		Owner->AddXP(Quest->XPReward);

		// Remove required item from inventory
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

	if (ADefaultPlayerController* PC = GetPC())
		PC->SetHUDQuestText(Quest->CompleteText.ToString());

	ActiveQuest = FActiveQuestState();
}

void UQuestManagerComponent::OnInventoryChanged()
{
	if (!ActiveQuest.Definition || ActiveQuest.bItemCollected) return;

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
		ActiveQuest.bItemCollected = true;
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

	const FText& Objective = ActiveQuest.bItemCollected
		? ActiveQuest.Definition->ReturnObjective
		: ActiveQuest.Definition->CollectObjective;

	PC->SetHUDQuestText(Objective.ToString());
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
