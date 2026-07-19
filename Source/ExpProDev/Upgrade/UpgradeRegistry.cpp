// No Rights Reserved @ Team Expedition

#include "Upgrade/UpgradeRegistry.h"
#include "Upgrade/UpgradeDefinition.h"

UUpgradeDefinition* UUpgradeRegistry::FindById(FName UpgradeId) const
{
	for (UUpgradeDefinition* Def : Upgrades)
	{
		if (Def && Def->UpgradeId == UpgradeId)
			return Def;
	}
	return nullptr;
}
