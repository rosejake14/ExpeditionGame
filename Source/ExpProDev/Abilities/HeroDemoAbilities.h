// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HeroAbility.h"
#include "HeroDemoAbilities.generated.h"

//
//  PLACEHOLDERS.
//
//  These exist to prove the ability pipeline end to end — granting, input, cooldown, charge and
//  HUD — while designers work out what the real kits are. All four placeholder heroes share
//  them. Replacing one is a data edit on a UHeroDefinition, not a code change here.
//
//  Each is deliberately the simplest thing that demonstrates its slot's gating rule.
//

/** Signature slot: a short forward dash on a cooldown, standing in for a weapon action. */
UCLASS()
class EXPPRODEV_API UHeroAbility_Dash : public UHeroAbility
{
	GENERATED_BODY()
public:
	UHeroAbility_Dash();

	/** Horizontal impulse strength. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float DashStrength = 1500.f;

	/** Slight upward component, so the dash clears small steps instead of catching on them. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float DashLift = 250.f;

protected:
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) override;
};

/** Addon slot: a self-heal on a longer cooldown, standing in for a secondary attack. */
UCLASS()
class EXPPRODEV_API UHeroAbility_HealBurst : public UHeroAbility
{
	GENERATED_BODY()
public:
	UHeroAbility_HealBurst();

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float HealAmount = 35.f;

protected:
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) override;
};

/**
 *  Ultimate slot: a radial damage burst, gated on a full charge meter rather than a cooldown.
 *  This is the one that proves kills fill the meter and activation spends it.
 */
UCLASS()
class EXPPRODEV_API UHeroAbility_ChargeBurst : public UHeroAbility
{
	GENERATED_BODY()
public:
	UHeroAbility_ChargeBurst();

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float BurstDamage = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float BurstRadius = 600.f;

protected:
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) override;
};
