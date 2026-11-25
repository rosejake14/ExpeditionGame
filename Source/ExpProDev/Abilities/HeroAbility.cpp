// No Rights Reserved @ Team Expedition

#include "Abilities/HeroAbility.h"

#include "Abilities/HeroAttributeSet.h"
#include "Abilities/HeroCooldownEffect.h"
#include "Abilities/HeroSupportEffects.h"
#include "Core/ExpProGameplayTags.h"
#include "ExpProDev.h"
#include "Hero/HeroCharacter.h"

#include "AbilitySystemComponent.h"

UHeroAbility::UHeroAbility()
{
	// Abilities run on the owning client only — this is a singleplayer project, and instancing
	// per-actor keeps per-ability state (cooldown timers, montage handles) simple.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	CooldownGameplayEffectClass = UHeroCooldownEffect::StaticClass();
}

FGameplayTag UHeroAbility::GetCooldownTagForSlot(EHeroAbilitySlot InSlot)
{
	switch (InSlot)
	{
	case EHeroAbilitySlot::Addon:    return ExpProTags::Cooldown_Addon;
	case EHeroAbilitySlot::Ultimate: return ExpProTags::Cooldown_Ultimate;
	case EHeroAbilitySlot::Power:
	default:                         return ExpProTags::Cooldown_Power;
	}
}

void UHeroAbility::FinishAbility(bool bWasCancelled)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

float UHeroAbility::GetScaledDamage(float BaseDamage) const
{
	const UAbilitySystemComponent* ASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
	const UHeroAttributeSet* Attributes = ASC ? ASC->GetSet<UHeroAttributeSet>() : nullptr;
	return Attributes ? BaseDamage * Attributes->GetDamageMultiplier() : BaseDamage;
}

AHeroCharacter* UHeroAbility::GetHeroCharacter(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return ActorInfo ? Cast<AHeroCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
}

const FGameplayTagContainer* UHeroAbility::GetCooldownTags() const
{
	// A charge-gated or zero-second ability has no cooldown to check at all. Returning null tells
	// GAS there is nothing blocking it, rather than leaving a tag that never gets applied.
	if (bChargeGated || CooldownSeconds <= 0.f)
	{
		return nullptr;
	}

	CooldownTags.Reset();
	CooldownTags.AddTag(GetCooldownTagForSlot(GrantedSlot));
	return &CooldownTags;
}

void UHeroAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (bChargeGated || CooldownSeconds <= 0.f)
	{
		return;
	}

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(
		CooldownGameplayEffectClass, GetAbilityLevel());
	if (!Spec.IsValid())
	{
		return;
	}

	// Duration and identity are both supplied here, which is what lets one shared cooldown
	// effect serve every ability in the game.
	Spec.Data->SetSetByCallerMagnitude(ExpProTags::Data_CooldownDuration, CooldownSeconds);
	Spec.Data->DynamicGrantedTags.AddTag(GetCooldownTagForSlot(GrantedSlot));

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
}

bool UHeroAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!bChargeGated)
	{
		return true;
	}

	// Ultimates are gated on a full meter instead of a cooldown.
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC) return false;

	const UHeroAttributeSet* Attributes = ASC->GetSet<UHeroAttributeSet>();
	if (!Attributes) return false;

	return Attributes->GetUltimateCharge() >= Attributes->GetMaxUltimateCharge();
}

void UHeroAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// CommitAbility runs the cost and the cooldown; if it fails, nothing should happen.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Spend the meter before doing anything else, so an ability that somehow re-enters cannot
	// fire twice off one charge.
	if (bChargeGated)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
		{
			if (const UHeroAttributeSet* Attributes = ASC->GetSet<UHeroAttributeSet>())
			{
				FGameplayEffectSpecHandle SpendSpec = MakeOutgoingGameplayEffectSpec(
					UUltimateSpendEffect::StaticClass(), GetAbilityLevel());
				if (SpendSpec.IsValid())
				{
					SpendSpec.Data->SetSetByCallerMagnitude(
						ExpProTags::Data_UltimateCharge, -Attributes->GetUltimateCharge());
					ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpendSpec);
				}
			}
		}
	}

	ExecuteHeroAbility(ActorInfo);
	OnAbilityActivated();

	// Long-running abilities end themselves through FinishAbility.
	if (bEndsImmediately)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
