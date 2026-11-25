// No Rights Reserved @ Team Expedition

#include "Abilities/HeroSupportEffects.h"

#include "Abilities/HeroAttributeSet.h"
#include "Core/ExpProGameplayTags.h"

namespace
{
	/** Every effect here is the same shape: instant, one SetByCaller modifier on one attribute. */
	FGameplayModifierInfo MakeSetByCallerModifier(const FGameplayAttribute& Attribute, const FGameplayTag& DataTag)
	{
		FSetByCallerFloat Magnitude;
		Magnitude.DataTag = DataTag;

		FGameplayModifierInfo Modifier;
		Modifier.Attribute        = Attribute;
		Modifier.ModifierOp       = EGameplayModOp::Additive;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Magnitude);
		return Modifier;
	}
}

UHealEffect::UHealEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Modifiers.Add(MakeSetByCallerModifier(UHeroAttributeSet::GetHealthAttribute(), ExpProTags::Data_Heal));
}

UUltimateChargeEffect::UUltimateChargeEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Modifiers.Add(MakeSetByCallerModifier(
		UHeroAttributeSet::GetUltimateChargeAttribute(), ExpProTags::Data_UltimateCharge));
}

UUltimateSpendEffect::UUltimateSpendEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	// Same tag as the grant — the caller passes a negative magnitude to spend.
	Modifiers.Add(MakeSetByCallerModifier(
		UHeroAttributeSet::GetUltimateChargeAttribute(), ExpProTags::Data_UltimateCharge));
}
