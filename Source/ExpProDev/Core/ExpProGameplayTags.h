// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 *  Every gameplay tag the C++ side of the project relies on, declared natively so the compiler
 *  catches a typo that a string literal never would. Tags declared here are also visible in the
 *  editor's tag picker without anyone having to author them into an ini.
 *
 *  Add a tag by declaring it here and defining it in ExpProGameplayTags.cpp — the two must match.
 */
namespace ExpProTags
{
	// ── Ability slots ─────────────────────────────────────────────────────
	// A hero has exactly three. UHeroDefinition maps an input action to one of these, and the
	// ability bar draws one widget per slot, so the set is deliberately fixed.

	/** The hero's weapon action. Gated by a cooldown standing in for reload time. */
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Slot_Signature);

	/** Secondary attack. Cooldown-gated, or uncapped when the ability has no cooldown effect. */
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Slot_Addon);

	/** Gated by UltimateCharge rather than a cooldown — see UHeroAttributeSet. */
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Slot_Ultimate);

	// ── SetByCaller magnitudes ────────────────────────────────────────────
	// Tags used to hand a runtime number to a GameplayEffect. The effect asset must name the
	// same tag in its SetByCaller modifier or the magnitude silently resolves to zero.

	/** Incoming damage, passed to GE_Damage by APlayerCharacter::RecieveDamage. */
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);

	/** Ultimate charge granted on an enemy kill, passed to UUltimateChargeEffect. */
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_UltimateCharge);

	/** Health restored, passed to UHealEffect. */
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Heal);

	/** Cooldown duration in seconds, passed to UHeroCooldownEffect. */
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CooldownDuration);

	// ── Cooldowns ─────────────────────────────────────────────────────────
	// One per slot. A hero only ever has one ability per slot, so the slot is enough to keep
	// separate cooldowns apart without a tag per ability.

	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Signature);
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Addon);
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Ultimate);

	// ── States ────────────────────────────────────────────────────────────

	/** Applied while the player is eliminated. Blocks ability activation. */
	EXPPRODEV_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
}
