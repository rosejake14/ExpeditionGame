// No Rights Reserved @ Team Expedition

#include "Hero/HeroRegistry.h"
#include "Hero/HeroDefinition.h"

UHeroDefinition* UHeroRegistry::FindById(FName HeroId) const
{
	for (UHeroDefinition* Def : Heroes)
	{
		if (Def && Def->HeroId == HeroId)
			return Def;
	}
	return nullptr;
}

UHeroDefinition* UHeroRegistry::GetDefaultHero() const
{
	for (UHeroDefinition* Def : Heroes)
	{
		if (Def)
			return Def;
	}
	return nullptr;
}
