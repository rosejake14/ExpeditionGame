// No Rights Reserved @ Team Expedition

#include "Abilities/HeroCooldownEffect.h"

#include "Core/ExpProGameplayTags.h"

UHeroCooldownEffect::UHeroCooldownEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// Filled in per-application by UHeroAbility::ApplyCooldown.
	FSetByCallerFloat DurationByCaller;
	DurationByCaller.DataTag = ExpProTags::Data_CooldownDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(DurationByCaller);

	// The tag identifying *which* cooldown this is comes from the spec's DynamicGrantedTags,
	// also set in ApplyCooldown — nothing is granted by the effect itself.
}
