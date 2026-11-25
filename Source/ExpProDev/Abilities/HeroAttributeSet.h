// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "HeroAttributeSet.generated.h"

/**
 *  Every numeric stat a hero owns, and the single source of truth for all of them.
 *  APlayerCharacter no longer stores health or damage scaling as plain floats — its getters
 *  proxy straight through to this set, so there is only ever one value to be wrong.
 *
 *  Damage is a *meta* attribute: incoming damage lands on it via UDamageEffect, and
 *  PostGameplayEffectExecute turns that into a Health change before zeroing it again. Nothing
 *  reads or persists Damage itself. Funnelling every hit through one attribute means armour,
 *  shields or damage-type rules can be added in exactly one place later.
 *
 *  Attributes are created and owned by APlayerCharacter alongside its ability system component.
 */
UCLASS()
class EXPPRODEV_API UHeroAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UHeroAttributeSet();

	/** Clamps values that arrive through the normal (non-instant) modifier path. */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** Handles the Damage meta attribute and re-clamps after instant effects execute. */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ── Health ────────────────────────────────────────────────────────────

	/** Current health. Hitting zero drives APlayerCharacter's elimination path. */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, Health)

	/** Upper bound for Health. Set from BaseMaxHealth plus any purchased health upgrades. */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, MaxHealth)

	// ── Stamina ───────────────────────────────────────────────────────────
	// Declared now so sprint can move onto it later. Nothing consumes it yet — sprint still
	// swaps MaxWalkSpeed directly, exactly as it did before this phase.

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, MaxStamina)

	// ── Combat ────────────────────────────────────────────────────────────

	/**
	 *  Multiplier on all outgoing damage — 1.0 is unmodified, upgrades add on top.
	 *  AWeapon reads this through APlayerCharacter::GetDamageMultiplier().
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData DamageMultiplier;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, DamageMultiplier)

	/** Metres per second the hero walks at. Reserved for per-hero movement in phase 2. */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, MoveSpeed)

	// ── Ultimate ──────────────────────────────────────────────────────────
	// Ultimates are charge-gated rather than cooldown-gated — see doc/HeroSystem.md §5.
	// Nothing grants charge yet; that arrives with the kill hook in phase 3.

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Ultimate")
	FGameplayAttributeData UltimateCharge;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, UltimateCharge)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Ultimate")
	FGameplayAttributeData MaxUltimateCharge;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, MaxUltimateCharge)

	// ── Meta ──────────────────────────────────────────────────────────────

	/**
	 *  Transient damage bucket. Only UDamageEffect writes to it, and PostGameplayEffectExecute
	 *  drains it into Health immediately. Never save, replicate or read this directly.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UHeroAttributeSet, Damage)
};
