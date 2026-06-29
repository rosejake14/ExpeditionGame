// No Rights Reserved @ Team Expedition

#include "Weapon/WeaponRegistry.h"
#include "Weapon/WeaponDefinition.h"

UWeaponDefinition* UWeaponRegistry::FindById(FName WeaponId) const
{
	for (UWeaponDefinition* Def : Weapons)
	{
		if (Def && Def->WeaponId == WeaponId)
			return Def;
	}
	return nullptr;
}
