// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "HeroAbility.generated.h"

/**
 *  Which of a hero's three ability slots an ability fills. Drives input, HUD position and
 *  cooldown tag. Main (Mouse 1) is weapon fire and is deliberately not a slot yet.
 */
UENUM(BlueprintType)
enum class EHeroAbilitySlot : uint8
{
	Power     UMETA(DisplayName = "Power"),
	Addon     UMETA(DisplayName = "Addon"),
	Ultimate  UMETA(DisplayName = "Ultimate")
};

/**
 *  Base class for every hero ability.
 *
 *  Two things it adds on top of UGameplayAbility:
 *
 *   1. **A data-driven cooldown.** Set CooldownSeconds and the shared UHeroCooldownEffect is
 *      applied with that duration, tagged for this ability's slot. No per-ability cooldown asset.
 *      Zero seconds means no cooldown at all — that is how "unlimited use" abilities are authored.
 *
 *   2. **Charge gating for ultimates.** Set bChargeGated and the ability refuses to activate
 *      until UltimateCharge has reached MaxUltimateCharge, then spends the whole meter. Ultimates
 *      build from kills rather than ticking down, so they do not use the cooldown path at all.
 *
 *  Presentation is left to Blueprint via OnAbilityActivated — montages, VFX and sound belong
 *  there, not here.
 */
UCLASS(Abstract, Blueprintable)
class EXPPRODEV_API UHeroAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UHeroAbility();

	// ── Presentation ──────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability")
	class UTexture2D* Icon = nullptr;

	// ── Behaviour ─────────────────────────────────────────────────────────

	/** Seconds before it can be used again. 0 = no cooldown. Ignored when bChargeGated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float CooldownSeconds = 0.f;

	/** Ultimates: needs a full charge meter, and spends it on activation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability")
	bool bChargeGated = false;

	/**
	 *  True: the ability ends in the same frame it executes. False: it stays active after
	 *  ExecuteHeroAbility, and the subclass ends it with FinishAbility (spins, channels, etc.).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability")
	bool bEndsImmediately = true;

	/** Blueprint presentation hook — montage, VFX, sound. Fired after the ability commits. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Ability")
	void OnAbilityActivated();

	// The end-of-ability BP hook is the engine's own "On End Ability" (K2_OnEndAbility), which
	// fires on normal end and on cancel. Stop looping VFX there.

	/** The cooldown tag for a given slot. Used by the HUD as well as the cooldown itself. */
	static FGameplayTag GetCooldownTagForSlot(EHeroAbilitySlot InSlot);

	/**
	 *  Called by AHeroCharacter::GrantAbilities on this ability's instance straight after it is
	 *  granted. The slot comes from where the hero definition put the ability, never from the
	 *  ability itself, so the cooldown tag always matches the HUD slot watching it.
	 */
	void SetGrantedSlot(EHeroAbilitySlot InSlot) { GrantedSlot = InSlot; }

	// ── UGameplayAbility ──────────────────────────────────────────────────

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	/** Ends a bEndsImmediately = false ability from inside the subclass. */
	void FinishAbility(bool bWasCancelled = false);

	/**
	 *  BaseDamage scaled by the caster's DamageMultiplier attribute. Enemies have no ability system,
	 *  so ability damage is delivered with ApplyDamage; a player target turns it into GE_Damage
	 *  through APlayerCharacter::RecieveDamage.
	 */
	float GetScaledDamage(float BaseDamage) const;

	/**
	 *  Where a subclass puts its actual effect. Called after the ability has committed, so the
	 *  cooldown is already running and any charge already spent — just do the thing.
	 */
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) {}

	/** The hero that activated this. Null if the ability is somehow running without one. */
	class AHeroCharacter* GetHeroCharacter(const FGameplayAbilityActorInfo* ActorInfo) const;

private:
	/** Set at grant time — see SetGrantedSlot. */
	EHeroAbilitySlot GrantedSlot = EHeroAbilitySlot::Power;

	/**
	 *  Rebuilt on demand rather than in the constructor, because the slot is only known once the
	 *  ability is granted. Mutable so the const GetCooldownTags can fill it and hand back a
	 *  stable pointer, which is what GAS expects.
	 */
	mutable FGameplayTagContainer CooldownTags;
};
