// No Rights Reserved @ Team Expedition

#include "Abilities/DamageEffect.h"

#include "Abilities/HeroAttributeSet.h"
#include "Core/ExpProGameplayTags.h"

UDamageEffect::UDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// The magnitude is supplied per-application by the caller rather than authored here — see
	// APlayerCharacter::RecieveDamage, which sets it from the engine's damage value.
	FSetByCallerFloat DamageMagnitude;
	DamageMagnitude.DataTag = ExpProTags::Data_Damage;

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute        = UHeroAttributeSet::GetDamageAttribute();
	DamageModifier.ModifierOp       = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(DamageMagnitude);

	Modifiers.Add(DamageModifier);
}
