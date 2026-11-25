// No Rights Reserved @ Team Expedition

#include "HUD/AbilitySlotWidget.h"

#include "Abilities/HeroAttributeSet.h"
#include "Hero/HeroCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void UAbilitySlotWidget::InitSlot(AHeroCharacter* InHero)
{
	Hero = InHero;

	if (const UHeroAbility* Ability = Hero ? Hero->GetAbilityForSlot(AbilitySlot) : nullptr)
	{
		bHasAbility  = true;
		AbilityName  = Ability->DisplayName;
		AbilityIcon  = Ability->Icon;
	}
	else
	{
		bHasAbility = false;
	}
}

void UAbilitySlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const UHeroAbility* Ability = Hero ? Hero->GetAbilityForSlot(AbilitySlot) : nullptr;
	if (!Ability)
	{
		bHasAbility = false;
		bIsReady = false;
		FillFraction = 0.f;
		CooldownRemaining = 0.f;
		return;
	}

	bHasAbility = true;

	UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent();
	if (!ASC) return;

	if (Ability->bChargeGated)
	{
		// Ultimates fill from kills rather than counting down.
		const UHeroAttributeSet* Attributes = ASC->GetSet<UHeroAttributeSet>();
		const float MaxCharge = Attributes ? Attributes->GetMaxUltimateCharge() : 0.f;

		FillFraction = MaxCharge > 0.f
			? FMath::Clamp(Attributes->GetUltimateCharge() / MaxCharge, 0.f, 1.f)
			: 0.f;
		CooldownRemaining = 0.f;
		bIsReady = FillFraction >= 1.f;
		return;
	}

	if (Ability->CooldownSeconds <= 0.f)
	{
		// An "unlimited use" ability — always ready, always full.
		FillFraction = 1.f;
		CooldownRemaining = 0.f;
		bIsReady = true;
		return;
	}

	// Ask the ability system how long the cooldown tag has left rather than tracking our own
	// timer, so the bar can never drift out of step with the actual gate.
	FGameplayTagContainer CooldownTag;
	CooldownTag.AddTag(UHeroAbility::GetCooldownTagForSlot(AbilitySlot));

	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag);
	const TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(Query);

	float LongestRemaining = 0.f;
	for (const float Remaining : TimesRemaining)
	{
		LongestRemaining = FMath::Max(LongestRemaining, Remaining);
	}

	CooldownRemaining = LongestRemaining;
	FillFraction = FMath::Clamp(1.f - (LongestRemaining / Ability->CooldownSeconds), 0.f, 1.f);
	bIsReady = LongestRemaining <= 0.f;
}
