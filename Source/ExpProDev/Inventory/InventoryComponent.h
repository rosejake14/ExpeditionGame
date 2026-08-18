// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	class UItemDefinition* ItemDef = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 0;

	// Non-null only for Weapon-type items
	UPROPERTY(BlueprintReadOnly)
	class AWeapon* WeaponInstance = nullptr;

	bool IsEmpty() const { return ItemDef == nullptr || Quantity <= 0; }
};

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EXPPRODEV_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UInventoryComponent();
	virtual void BeginPlay() override;

	// Number of hotbar slots — also determines the column count in the full inventory grid.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 HotbarSlotCount = 4;

	// Number of overflow slots (rows beyond the hotbar row). Total inventory = HotbarSlotCount + OverflowSlotCount.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 OverflowSlotCount = 8;

	bool AddItem(class UItemDefinition* ItemDef, int32 Quantity = 1, class AWeapon* WeaponInstance = nullptr);
	bool RemoveItem(int32 SlotIndex, int32 Quantity = 1);
	// TECH_DEBT(TD-STUB-4): SwapSlots has no callers — the inventory screen is display-only. There
	// is no DropSlot either, so items can only leave the inventory by selling or dying.
	void SwapSlots(int32 IndexA, int32 IndexB);

	const FInventorySlot& GetSlot(int32 Index) const;
	int32 GetTotalSlotCount() const { return HotbarSlotCount + OverflowSlotCount; }
	int32 GetHotbarSlotCount() const { return HotbarSlotCount; }

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 ActiveHotbarIndex = 0;
	void SetActiveHotbarIndex(int32 Index);

	// Broadcast whenever slots change — UI binds to this
	// TECH_DEBT(TD-ARCH-23): non-dynamic multicast, so Blueprints can't bind to it and listeners
	// have no unbind discipline (UHotbarWidget binds twice — see TD-BUG-10).
	FOnInventoryChanged OnInventoryChanged;

private:
	TArray<FInventorySlot> Slots;

	int32 FindExistingStack(class UItemDefinition* ItemDef) const;
	int32 FindFirstEmptySlot() const;
};
