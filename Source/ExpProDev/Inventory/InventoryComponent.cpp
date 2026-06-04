// No Rights Reserved @ Team Expedition

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	Slots.SetNum(HotbarSlotCount * 3);
}

bool UInventoryComponent::AddItem(UItemDefinition* ItemDef, int32 Quantity, AWeapon* WeaponInstance)
{
	if (!ItemDef || Quantity <= 0) return false;

	// Try stacking onto an existing slot first
	if (ItemDef->MaxStackSize > 1)
	{
		int32 StackIndex = FindExistingStack(ItemDef);
		if (StackIndex != INDEX_NONE)
		{
			FInventorySlot& Slot = Slots[StackIndex];
			int32 ToAdd = FMath::Min(ItemDef->MaxStackSize - Slot.Quantity, Quantity);
			Slot.Quantity += ToAdd;
			OnInventoryChanged.Broadcast();
			return true;
		}
	}

	int32 EmptyIndex = FindFirstEmptySlot();
	if (EmptyIndex == INDEX_NONE) return false;

	Slots[EmptyIndex].ItemDef = ItemDef;
	Slots[EmptyIndex].Quantity = Quantity;
	Slots[EmptyIndex].WeaponInstance = WeaponInstance;
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Quantity)
{
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty()) return false;

	FInventorySlot& Slot = Slots[SlotIndex];
	Slot.Quantity -= Quantity;
	if (Slot.Quantity <= 0)
	{
		Slot.ItemDef = nullptr;
		Slot.Quantity = 0;
		Slot.WeaponInstance = nullptr;
	}
	OnInventoryChanged.Broadcast();
	return true;
}

void UInventoryComponent::SwapSlots(int32 IndexA, int32 IndexB)
{
	if (!Slots.IsValidIndex(IndexA) || !Slots.IsValidIndex(IndexB)) return;
	Slots.Swap(IndexA, IndexB);
	OnInventoryChanged.Broadcast();
}

const FInventorySlot& UInventoryComponent::GetSlot(int32 Index) const
{
	static FInventorySlot EmptySlot;
	if (!Slots.IsValidIndex(Index)) return EmptySlot;
	return Slots[Index];
}

void UInventoryComponent::SetActiveHotbarIndex(int32 Index)
{
	if (Index < 0 || Index >= HotbarSlotCount) return;
	ActiveHotbarIndex = Index;
	OnInventoryChanged.Broadcast();
}

int32 UInventoryComponent::FindExistingStack(UItemDefinition* ItemDef) const
{
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].ItemDef == ItemDef && Slots[i].Quantity < ItemDef->MaxStackSize)
			return i;
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::FindFirstEmptySlot() const
{
	// Fill hotbar slots before overflow slots
	for (int32 i = 0; i < HotbarSlotCount; i++)
	{
		if (Slots[i].IsEmpty()) return i;
	}
	for (int32 i = HotbarSlotCount; i < Slots.Num(); i++)
	{
		if (Slots[i].IsEmpty()) return i;
	}
	return INDEX_NONE;
}
