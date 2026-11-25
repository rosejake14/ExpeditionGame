// No Rights Reserved @ Team Expedition

#include "Abilities/HeroDemoAbilities.h"

#include "Abilities/HeroSupportEffects.h"
#include "Core/ExpProGameplayTags.h"
#include "ExpProDev.h"
#include "Hero/HeroCharacter.h"

#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

// ── Dash ──────────────────────────────────────────────────────────────────

UHeroAbility_Dash::UHeroAbility_Dash()
{
	CooldownSeconds = 5.f;
	DisplayName = NSLOCTEXT("HeroAbilities", "Dash", "Dash");
}

void UHeroAbility_Dash::ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo)
{
	AHeroCharacter* Hero = GetHeroCharacter(ActorInfo);
	if (!Hero) return;

	const FVector Direction = Hero->GetActorForwardVector() * DashStrength + FVector(0.f, 0.f, DashLift);

	// bXYOverride/bZOverride true so the dash reads consistently regardless of current momentum.
	Hero->LaunchCharacter(Direction, true, true);
}

// ── Heal burst ────────────────────────────────────────────────────────────

UHeroAbility_HealBurst::UHeroAbility_HealBurst()
{
	CooldownSeconds = 15.f;
	DisplayName = NSLOCTEXT("HeroAbilities", "HealBurst", "Heal Burst");
}

void UHeroAbility_HealBurst::ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo)
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle Spec =
		ASC->MakeOutgoingSpec(UHealEffect::StaticClass(), GetAbilityLevel(), Context);
	if (!Spec.IsValid()) return;

	// The attribute set clamps to MaxHealth, so overhealing is harmless.
	Spec.Data->SetSetByCallerMagnitude(ExpProTags::Data_Heal, HealAmount);
	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
}

// ── Charge burst (ultimate) ───────────────────────────────────────────────

UHeroAbility_ChargeBurst::UHeroAbility_ChargeBurst()
{
	bChargeGated = true;
	DisplayName = NSLOCTEXT("HeroAbilities", "ChargeBurst", "Charge Burst");
}

void UHeroAbility_ChargeBurst::ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo)
{
	AHeroCharacter* Hero = GetHeroCharacter(ActorInfo);
	if (!Hero) return;

	// Ignore the caster so the ultimate can't kill its own hero.
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Hero);

	UGameplayStatics::ApplyRadialDamage(
		Hero,
		GetScaledDamage(BurstDamage),
		Hero->GetActorLocation(),
		BurstRadius,
		nullptr,
		IgnoreActors,
		Hero,
		Hero->GetController(),
		/*bDoFullDamage*/ true);

	UE_LOG(LogExpProDev, Log, TEXT("HeroAbility: charge burst fired (%.0f damage, %.0f radius)."),
		BurstDamage, BurstRadius);
}
