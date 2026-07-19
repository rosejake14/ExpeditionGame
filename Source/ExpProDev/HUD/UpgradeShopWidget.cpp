// No Rights Reserved @ Team Expedition

#include "HUD/UpgradeShopWidget.h"
#include "Economy/EconomySubsystem.h"
#include "Upgrade/UpgradeDefinition.h"
#include "Upgrade/UpgradeRegistry.h"

TArray<FShopEntry> UUpgradeShopWidget::BuildEntries() const
{
	TArray<FShopEntry> Entries;

	UEconomySubsystem* Econ = GetEconomy();
	if (!Registry || !Econ)
	{
		if (!Registry)
			UE_LOG(LogTemp, Error, TEXT("UUpgradeShopWidget: Registry is unset — assign DA_UpgradeRegistry in the widget BP."));
		return Entries;
	}

	const int32 Balance = Econ->GetBalance();

	for (UUpgradeDefinition* Def : Registry->Upgrades)
	{
		if (!Def) continue;

		const int32 Count  = Econ->GetUpgradeCount(Def->UpgradeId);
		const bool  bMaxed = Def->IsMaxed(Count);
		const int32 Cost   = Def->GetCostForNextPurchase(Count);

		FShopEntry Entry;
		Entry.Id          = Def->UpgradeId;
		Entry.Name        = Def->DisplayName;
		Entry.Description = Def->Description;
		Entry.Icon        = Def->Icon;

		if (Def->MaxPurchases > 0)
			Entry.StatusText = FText::Format(
				FText::FromString(TEXT("Level: {0} / {1}")),
				FText::AsNumber(Count), FText::AsNumber(Def->MaxPurchases));
		else
			Entry.StatusText = FText::Format(
				FText::FromString(TEXT("Level: {0}")), FText::AsNumber(Count));

		Entry.CostText    = bMaxed
			? FText::FromString(TEXT("MAX"))
			: FText::Format(FText::FromString(TEXT("{0} DOS$")), FText::AsNumber(Cost));
		Entry.bBuyEnabled = !bMaxed && Balance >= Cost;

		Entries.Add(Entry);
	}

	return Entries;
}

void UUpgradeShopWidget::PurchaseEntry(FName Id)
{
	if (UEconomySubsystem* Econ = GetEconomy())
		Econ->TryPurchaseUpgrade(Id, Registry);
}
