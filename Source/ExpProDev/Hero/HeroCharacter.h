// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HeroAbility.h"
#include "Character/PlayerCharacter.h"
#include "Engine/TimerHandle.h"
#include "GameplayAbilitySpec.h"
#include "HeroCharacter.generated.h"

class UHeroDefinition;
class UInputAction;

/**
 *  The pawn every hero uses. It holds no per-hero logic of its own — it reads a
 *  UHeroDefinition and applies it, so all four heroes are the same class with different data.
 *
 *  ADefaultGameMode spawns this deferred, calls SetHeroDefinition, then finishes the spawn, so
 *  appearance is already correct before BeginPlay and possession run.
 *
 *  Subclass this only for a hero that needs genuinely bespoke C++ — see doc/HeroSystem.md §4.
 */
UCLASS()
class EXPPRODEV_API AHeroCharacter : public APlayerCharacter
{
	GENERATED_BODY()
public:
	/**
	 *  Assigns the hero this pawn represents and immediately applies its appearance.
	 *  Call between SpawnActorDeferred and FinishSpawningActor.
	 */
	void SetHeroDefinition(UHeroDefinition* InDefinition);

	UFUNCTION(BlueprintPure, Category = "Hero")
	UHeroDefinition* GetHeroDefinition() const { return HeroDefinition; }

	/** The ability filling a slot, or null if this hero has none there. Used by the HUD. */
	UFUNCTION(BlueprintPure, Category = "Hero")
	UHeroAbility* GetAbilityForSlot(EHeroAbilitySlot InSlot) const;

	//
	// Ability input. These are the same three keys for every hero, so they live on the pawn
	// rather than being repeated in all four hero definitions.
	//

	/** E. */
	UPROPERTY(EditAnywhere, Category = "Hero|Input")
	UInputAction* PowerAbilityAction = nullptr;

	//
	// Camera zoom, shared by every ability (C++ or Blueprint).
	//

	/**
	 *  Eases the camera arm out to Multiplier x its normal length over BlendTime, holds, then eases
	 *  back over the last BlendTime of Duration. Calling again mid-zoom restarts from the current
	 *  length, so there is no jump. BlendTime is clamped to half of Duration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void ZoomCamera(float Multiplier, float Duration, float BlendTime = 0.25f);

	/** Eases straight back to the normal length from wherever the zoom currently is. */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void StopCameraZoom(float BlendTime = 0.25f);

	/**
	 *  Sets the arm to Multiplier x its normal length right now, cancelling any timed zoom. For
	 *  abilities whose zoom follows something live (e.g. height); call every frame, then
	 *  StopCameraZoom when done.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void SetCameraZoom(float Multiplier);

	UPROPERTY(EditAnywhere, Category = "Hero|Input")
	UInputAction* AddonAbilityAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Hero|Input")
	UInputAction* UltimateAbilityAction = nullptr;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	/** Takes starting stats from the definition rather than the inherited BaseMaxHealth. */
	virtual void InitialiseAttributes() override;

	/** Mesh, animation, material, scale and capsule. Anything unset is left as the pawn's own. */
	void ApplyAppearance();

	/** Gives the three abilities the definition names, remembering each handle for input. */
	void GrantAbilities();

	/** Spawns and equips the definition's StartingWeapon, unless something is already equipped. */
	void EquipStartingWeapon();

	/** Input handlers — one per slot, each activating that slot's granted spec. */
	void ActivatePower();
	void ActivateAddon();
	void ActivateUltimate();

	/** Shared body of the three handlers above. */
	void ActivateSlot(EHeroAbilitySlot InSlot);

private:
	UPROPERTY()
	UHeroDefinition* HeroDefinition = nullptr;

	/**
	 *  Granted ability handles, indexed by EHeroAbilitySlot. Activating by handle rather than by
	 *  tag keeps input wiring independent of how an ability tags itself.
	 */
	FGameplayAbilitySpecHandle AbilityHandles[3];

	// ── Camera zoom state ─────────────────────────────────────────────────

	/** One per-frame step of the active zoom. Re-arms itself until the zoom is done. */
	void StepCameraZoom();

	UPROPERTY()
	class USpringArmComponent* ZoomArm = nullptr;

	FTimerHandle ZoomTimer;

	/** The arm's normal length, captured on the first zoom so repeated zooms can't drift. <0 = not yet. */
	float BaseArmLength = -1.f;

	float ZoomStartLength = 0.f;   // length when the current zoom began
	float ZoomPeakLength = 0.f;    // held length
	float ZoomReturnFrom = 0.f;    // length the return blend starts from
	float ZoomDuration = 0.f;
	float ZoomBlend = 0.f;
	float ZoomElapsed = 0.f;
	float ZoomLastTime = 0.f;
};
