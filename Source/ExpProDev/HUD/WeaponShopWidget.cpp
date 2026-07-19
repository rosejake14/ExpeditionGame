// No Rights Reserved @ Team Expedition

#include "HUD/WeaponShopWidget.h"
#include "Economy/EconomySubsystem.h"
#include "Weapon/WeaponDefinition.h"
#include "Weapon/WeaponRegistry.h"

TArray<FShopEntry> UWeaponShopWidget::BuildEntries() const
{
	TArray<FShopEntry> Entries;

	UEconomySubsystem* Econ = GetEconomy();
	if (!Registry || !Econ)
	{
		if (!Registry)
			UE_LOG(LogTemp, Error, TEXT("UWeaponShopWidget: Registry is unset — assign DA_WeaponRegistry in the widget BP."));
		return Entries;
	}

	const int32 Balance = Econ->GetBalance();

	for (UWeaponDefinition* Def : Registry->Weapons)
	{
		if (!Def) continue;

		const int32 Owned = Econ->GetWeaponOwnedCount(Def->WeaponId);

		FShopEntry Entry;
		Entry.Id          = Def->WeaponId;
		Entry.Name        = Def->DisplayName;
		Entry.Description = Def->Description;
		Entry.Icon        = Def->Icon;
		Entry.StatusText  = FText::Format(FText::FromString(TEXT("Owned: {0}")), FText::AsNumber(Owned));
		Entry.CostText    = FText::Format(FText::FromString(TEXT("{0} DOS$")), FText::AsNumber(Def->Cost));
		Entry.bBuyEnabled = Balance >= Def->Cost;

		Entries.Add(Entry);
	}

	return Entries;
}

void UWeaponShopWidget::PurchaseEntry(FName Id)
{
	if (UEconomySubsystem* Econ = GetEconomy())
		Econ->TryPurchaseWeapon(Id, Registry);
}
