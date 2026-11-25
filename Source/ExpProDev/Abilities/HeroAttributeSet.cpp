// No Rights Reserved @ Team Expedition

#include "Abilities/HeroAttributeSet.h"

#include "GameplayEffectExtension.h"

UHeroAttributeSet::UHeroAttributeSet()
{
	// Sane defaults so an attribute set that somehow escapes initialisation is still usable
	// rather than silently zeroed. APlayerCharacter overwrites all of these on possession.
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitDamageMultiplier(1.f);
	InitMoveSpeed(600.f);
	InitUltimateCharge(0.f);
	InitMaxUltimateCharge(100.f);
	InitDamage(0.f);
}

void UHeroAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetUltimateChargeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxUltimateCharge());
	}
}

void UHeroAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Damage is a meta attribute — drain it into Health and reset it in the same breath, so it
	// never accumulates across hits.
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float IncomingDamage = GetDamage();
		SetDamage(0.f);

		if (IncomingDamage > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - IncomingDamage, 0.f, GetMaxHealth()));
		}
		return;
	}

	// PreAttributeChange does not see instant effects modifying a base value, so anything that
	// can be driven directly by an effect is clamped again here.
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetUltimateChargeAttribute())
	{
		SetUltimateCharge(FMath::Clamp(GetUltimateCharge(), 0.f, GetMaxUltimateCharge()));
	}
}
