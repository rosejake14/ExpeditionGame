// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HeroDefinition.generated.h"

/**
 *  Everything that makes one hero different from another. Adding a hero should mean authoring
 *  one of these assets and adding it to the UHeroRegistry — no C++.
 *
 *  HeroClass exists as an escape hatch for a hero that genuinely needs bespoke code, but reach
 *  for an ability or an attribute first; every hero drafted so far is expressible as pure data.
 *
 *  Appearance fields are all "leave alone if unset": a null mesh, a null material, a zero scale
 *  or a zero capsule dimension means the pawn keeps whatever its own Blueprint defines. That way
 *  a half-filled definition degrades to the existing character instead of a broken one.
 *
 *  Stat fields are always applied, and their defaults match what APlayerCharacter used before
 *  heroes existed — so a definition left at its defaults plays exactly like the old player.
 */
UCLASS(BlueprintType)
class EXPPRODEV_API UHeroDefinition : public UDataAsset
{
	GENERATED_BODY()
public:
	// ── Identity ──────────────────────────────────────────────────────────

	/** Unique save key. Never rename this once a save has stored it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	FName HeroId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	FText DisplayName;

	/** "Tank", "Damage" — shown on the select screen. Free text, deliberately not an enum. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	FText RoleLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	class UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	class UTexture2D* Portrait = nullptr;

	// ── Pawn ──────────────────────────────────────────────────────────────

	/** Pawn class to spawn. Leave as the shared AHeroCharacter unless this hero needs bespoke C++. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Pawn")
	TSubclassOf<class AHeroCharacter> HeroClass;

	// ── Appearance (null / 0 = keep the pawn's own) ───────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Appearance")
	class USkeletalMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Appearance")
	TSubclassOf<class UAnimInstance> AnimClass;

	/** Applied to every material slot on the mesh — how the placeholder heroes are told apart. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Appearance")
	class UMaterialInterface* OverrideMaterial = nullptr;

	/** Uniform mesh scale. 0 keeps the pawn's own — this is the "short and heavy" dial. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Appearance", meta = (ClampMin = 0.0))
	float MeshScale = 0.f;

	/** 0 keeps the pawn's own capsule. Change alongside MeshScale or the hero floats or sinks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Appearance", meta = (ClampMin = 0.0))
	float CapsuleHalfHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Appearance", meta = (ClampMin = 0.0))
	float CapsuleRadius = 0.f;

	// ── Stats (always applied) ────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats", meta = (ClampMin = 1.0))
	float BaseMaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats", meta = (ClampMin = 1.0))
	float BaseMaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats", meta = (ClampMin = 1.0))
	float BaseWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats", meta = (ClampMin = 1.0))
	float BaseSprintSpeed = 1000.f;

	/** 1.0 is unmodified damage. Upgrades add on top of this. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats", meta = (ClampMin = 0.1))
	float BaseDamageMultiplier = 1.f;

	/** How much kill charge the ultimate needs. Consumed in phase 3. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats", meta = (ClampMin = 1.0))
	float BaseMaxUltimateCharge = 100.f;

	// ── Abilities ─────────────────────────────────────────────────────────
	// Typed as the GAS base rather than UHeroAbility, which does not exist until phase 3.
	// UHeroAbility will derive from UGameplayAbility, so assets assigned here stay valid.

	/** E — the hero's unique Power ability. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Abilities")
	TSubclassOf<class UGameplayAbility> PowerAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Abilities")
	TSubclassOf<class UGameplayAbility> AddonAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Abilities")
	TSubclassOf<class UGameplayAbility> UltimateAbility;

	// The keys these three are bound to are the same for every hero, so they live on
	// AHeroCharacter rather than being repeated in each definition.

	// ── Loadout & progression ─────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Loadout")
	TSubclassOf<class AWeapon> StartingWeapon;

	/** This hero's own upgrade shop contents. Wired up in phase 5. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Progression")
	class UUpgradeRegistry* UpgradeTree = nullptr;

	// ── Unlocking (enforced in phase 6) ───────────────────────────────────

	/** Account level required before this hero can be bought at all. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Unlock", meta = (ClampMin = 1))
	int32 RequiredAccountLevel = 1;

	/** DOS$ price once the level gate is met. 0 means free — use for the starter hero. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Unlock", meta = (ClampMin = 0))
	int32 UnlockCost = 0;

	/** True for a hero that costs nothing and has no level gate. */
	bool IsFreeStarter() const { return UnlockCost <= 0 && RequiredAccountLevel <= 1; }
};
