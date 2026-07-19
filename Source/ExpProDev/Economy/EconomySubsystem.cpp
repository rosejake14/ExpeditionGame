// No Rights Reserved @ Team Expedition

#include "Economy/EconomySubsystem.h"
#include "Save/ExpProSaveGame.h"
#include "Save/SaveGameSubsystem.h"
#include "Upgrade/UpgradeRegistry.h"
#include "Upgrade/UpgradeDefinition.h"
#include "Weapon/WeaponRegistry.h"
#include "Weapon/WeaponDefinition.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Kismet/GameplayStatics.h"

UEconomySubsystem* UEconomySubsystem::Get(const UObject* WorldContext)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	return GI ? GI->GetSubsystem<UEconomySubsystem>() : nullptr;
}

int32 UEconomySubsystem::GetBalance() const
{
	const UExpProSaveGame* Save = USaveGameSubsystem::LoadActiveSlot(this);
	return Save ? Save->DOSCoins : 0;
}

int32 UEconomySubsystem::GetUpgradeCount(FName UpgradeId) const
{
	const UExpProSaveGame* Save = USaveGameSubsystem::LoadActiveSlot(this);
	return Save ? Save->PurchasedUpgrades.FindRef(UpgradeId) : 0;
}

int32 UEconomySubsystem::GetWeaponOwnedCount(FName WeaponId) const
{
	const UExpProSaveGame* Save = USaveGameSubsystem::LoadActiveSlot(this);
	return Save ? Save->PurchasedWeapons.FindRef(WeaponId) : 0;
}

bool UEconomySubsystem::TryPurchaseUpgrade(FName UpgradeId, const UUpgradeRegistry* Registry)
{
	if (!Registry) return false;

	UUpgradeDefinition* Def = Registry->FindById(UpgradeId);
	if (!Def) return false;

	const int32 Count = GetUpgradeCount(UpgradeId);
	const int32 Cost  = Def->GetCostForNextPurchase(Count);
	if (Def->IsMaxed(Count) || GetBalance() < Cost) return false;

	int32 NewBalance = 0;
	USaveGameSubsystem::MutateActiveSlot(this, [UpgradeId, Cost, &NewBalance](UExpProSaveGame& Save)
	{
		Save.DOSCoins -= Cost;
		Save.PurchasedUpgrades.FindOrAdd(UpgradeId)++;
		NewBalance = Save.DOSCoins;
	});

	OnPurchaseCompleted.Broadcast(UpgradeId);
	OnBalanceChanged.Broadcast(NewBalance);
	return true;
}

bool UEconomySubsystem::TryPurchaseWeapon(FName WeaponId, const UWeaponRegistry* Registry)
{
	if (!Registry) return false;

	UWeaponDefinition* Def = Registry->FindById(WeaponId);
	if (!Def) return false;

	const int32 Cost = Def->Cost;
	if (GetBalance() < Cost) return false;

	int32 NewBalance = 0;
	USaveGameSubsystem::MutateActiveSlot(this, [WeaponId, Cost, &NewBalance](UExpProSaveGame& Save)
	{
		Save.DOSCoins -= Cost;
		Save.PurchasedWeapons.FindOrAdd(WeaponId)++;
		NewBalance = Save.DOSCoins;
	});

	OnPurchaseCompleted.Broadcast(WeaponId);
	OnBalanceChanged.Broadcast(NewBalance);
	return true;
}

FSellResult UEconomySubsystem::SellLoot(UInventoryComponent* Inventory)
{
	FSellResult Result;
	if (!Inventory)
	{
		Result.NewBalance = GetBalance();
		return Result;
	}

	// Build sell entries from all Loot-type slots.
	for (int32 i = 0; i < Inventory->GetTotalSlotCount(); ++i)
	{
		const FInventorySlot& Slot = Inventory->GetSlot(i);
		if (Slot.IsEmpty() || !Slot.ItemDef) continue;
		if (Slot.ItemDef->ItemType != EItemType::Loot) continue;

		FSellEntry Entry;
		Entry.Icon        = Slot.ItemDef->Icon;
		Entry.Quantity    = Slot.Quantity;
		Entry.CoinsEarned = FMath::RoundToInt(Slot.ItemDef->BaseValue) * Slot.Quantity;
		Result.TotalEarned += Entry.CoinsEarned;
		Result.Entries.Add(Entry);
	}

	// Remove the loot slots (reverse so indices stay valid).
	for (int32 i = Inventory->GetTotalSlotCount() - 1; i >= 0; --i)
	{
		const FInventorySlot& Slot = Inventory->GetSlot(i);
		if (!Slot.IsEmpty() && Slot.ItemDef && Slot.ItemDef->ItemType == EItemType::Loot)
		{
			Inventory->RemoveItem(i, Slot.Quantity);
		}
	}

	const int32 Earned = Result.TotalEarned;
	int32 NewBalance = 0;
	USaveGameSubsystem::MutateActiveSlot(this, [Earned, &NewBalance](UExpProSaveGame& Save)
	{
		Save.DOSCoins += Earned;
		NewBalance = Save.DOSCoins;
	});

	Result.NewBalance = NewBalance;
	OnBalanceChanged.Broadcast(NewBalance);
	return Result;
}
