// No Rights Reserved @ Team Expedition

#include "Upgrade/UpgradeManagerComponent.h"
#include "Upgrade/UpgradeDefinition.h"
#include "Upgrade/UpgradeRegistry.h"

void UUpgradeManagerComponent::LoadUpgrades(const TMap<FName, int32>& Saved)
{
	PurchasedUpgrades = Saved;
}

int32 UUpgradeManagerComponent::GetPurchaseCount(FName UpgradeId) const
{
	return PurchasedUpgrades.FindRef(UpgradeId);
}

float UUpgradeManagerComponent::GetHealthBonus() const
{
	if (!Registry) return 0.f;
	float Total = 0.f;
	for (const UUpgradeDefinition* Def : Registry->Upgrades)
	{
		if (Def && Def->UpgradeType == EUpgradeType::HealthBoost)
			Total += Def->ValuePerLevel * GetPurchaseCount(Def->UpgradeId);
	}
	return Total;
}

float UUpgradeManagerComponent::GetDamageMultiplierBonus() const
{
	if (!Registry) return 0.f;
	float Total = 0.f;
	for (const UUpgradeDefinition* Def : Registry->Upgrades)
	{
		if (Def && Def->UpgradeType == EUpgradeType::DamageBoost)
			Total += Def->ValuePerLevel * GetPurchaseCount(Def->UpgradeId);
	}
	return Total;
}
