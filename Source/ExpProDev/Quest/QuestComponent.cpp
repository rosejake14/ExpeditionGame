// No Rights Reserved @ Team Expedition

#include "Quest/QuestComponent.h"
#include "Character/PlayerCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "PlayerController/DefaultPlayerController.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner());
	if (!Owner) return;

	PlayerController = Cast<ADefaultPlayerController>(Owner->GetController());

	UInventoryComponent* Inv = Owner->GetInventory();
	if (Inv)
	{
		Inv->OnInventoryChanged.AddUObject(this, &UQuestComponent::OnInventoryChanged);
	}

	UpdateHUD();
}

void UQuestComponent::OnInventoryChanged()
{
	if (bItemCollected || !RequiredItem) return;

	APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner());
	if (!Owner) return;

	UInventoryComponent* Inv = Owner->GetInventory();
	if (!Inv) return;

	for (int32 i = 0; i < Inv->GetTotalSlotCount(); ++i)
	{
		const FInventorySlot& Slot = Inv->GetSlot(i);
		if (Slot.ItemDef == RequiredItem && Slot.Quantity > 0)
		{
			bItemCollected = true;
			UpdateHUD();
			return;
		}
	}
}

void UQuestComponent::NotifyQuestComplete()
{
	APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner());
	if (Owner && RequiredItem)
	{
		UInventoryComponent* Inv = Owner->GetInventory();
		if (Inv)
		{
			for (int32 i = 0; i < Inv->GetTotalSlotCount(); ++i)
			{
				const FInventorySlot& Slot = Inv->GetSlot(i);
				if (Slot.ItemDef == RequiredItem && Slot.Quantity > 0)
				{
					Inv->RemoveItem(i, 1);
					break;
				}
			}
		}
	}

	if (!PlayerController)
	{
		if (Owner) PlayerController = Cast<ADefaultPlayerController>(Owner->GetController());
	}

	if (PlayerController)
	{
		PlayerController->SetHUDQuestText(TEXT("Quest Completed"));
	}
}

void UQuestComponent::UpdateHUD()
{
	if (!PlayerController)
	{
		ACharacter* Owner = Cast<ACharacter>(GetOwner());
		if (Owner) PlayerController = Cast<ADefaultPlayerController>(Owner->GetController());
	}

	if (!PlayerController) return;

	const FString Text = bItemCollected ? TEXT("Return Item") : TEXT("Collect Item");
	PlayerController->SetHUDQuestText(Text);
}
