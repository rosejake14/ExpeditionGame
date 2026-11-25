// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HeroAbility.h"
#include "Engine/TimerHandle.h"
#include "BombshellAbilities.generated.h"

//
//  Bombshell's kit. Spec: doc/hero/HERO_BOMBSHELL_ABILITIES.md.
//  Main (M1) is the equipped weapon, Addon (F) is empty for now.
//

/**
 *  Power (E). The mesh spins SpinCount full turns at a constant SpinSpeed while the equipped
 *  weapon fires ShotsPerSecond times per second in whatever direction the body faces — a flat
 *  ring of shots. Only the mesh's relative yaw turns: the actor and camera are untouched, and the
 *  player can keep walking and looking around.
 *
 *  Shots go through AWeapon::Fire, so the weapon's own sound, trace, damage and DamageMultiplier
 *  scaling all apply, and swapping the weapon (e.g. to real dual pistols) needs no change here.
 *
 *  Long-running (bEndsImmediately = false): a looping timer drives the spin and FinishAbility
 *  ends it. All cleanup lives in EndAbility so death or cancellation restores rotation too.
 */
UCLASS()
class EXPPRODEV_API UHeroAbility_SpinShoot : public UHeroAbility
{
	GENERATED_BODY()
public:
	UHeroAbility_SpinShoot();

	/** X — full 360 degree turns. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1))
	int32 SpinCount = 2;

	/** Y — constant spin speed, degrees per second. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float SpinSpeed = 720.f;

	/** Z — shots fired per second during the spin. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.1))
	float ShotsPerSecond = 10.f;

	/** Distance from the weapon to each shot's target point. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float ShotRange = 3000.f;

	/** Camera pulls back to this multiple of its normal distance during the spin. 1 = no zoom. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float ZoomMultiplier = 2.f;

	/** Seconds to ease out at the start, and back in at the end. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float ZoomBlendTime = 0.25f;

	/** Refuses to activate (and so never starts the cooldown) without an equipped weapon. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) override;

private:
	/** One timer step: advance yaw by real elapsed time, fire any shots now due, end when done. */
	void StepSpin();

	void FireShot();

	UPROPERTY()
	class AHeroCharacter* SpinningHero = nullptr;

	FTimerHandle SpinTimer;

	float Elapsed = 0.f;
	float LastStepTime = 0.f;
	float ShotAccumulator = 0.f;

	/** Mesh relative rotation before the spin. Restored in EndAbility. */
	FRotator SavedMeshRotation = FRotator::ZeroRotator;

	/** The hero's camera arm, re-parented to the capsule during the spin. Restored in EndAbility. */
	UPROPERTY()
	class USpringArmComponent* SpinArm = nullptr;

	UPROPERTY()
	class USceneComponent* SavedArmParent = nullptr;

	FName SavedArmSocket = NAME_None;
	FTransform SavedArmRelative = FTransform::Identity;
};

/**
 *  Ultimate (Q). A bomb goes off at the hero's feet: radial damage to everything in BlastRadius
 *  except the hero, full damage across the radius, and the hero is launched straight up.
 *
 *  Long-running: stays active until the hero lands, zooming the camera out with height above the
 *  ground (1x on the ground, MaxZoomMultiplier at MaxZoomHeight).
 *
 *  The bomb itself is presentation only — spawn its mesh, VFX and sound from OnAbilityActivated
 *  in a Blueprint subclass. It explodes the frame it appears, so there is no C++ bomb actor.
 */
UCLASS()
class EXPPRODEV_API UHeroAbility_BombKick : public UHeroAbility
{
	GENERATED_BODY()
public:
	UHeroAbility_BombKick();

	/** Base damage, before the hero's DamageMultiplier. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float BlastDamage = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float BlastRadius = 800.f;

	/** Upward launch velocity. Horizontal momentum is kept. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float LaunchStrength = 1400.f;

	/** Camera zoom at (or above) MaxZoomHeight. Scales down to 1x on the ground. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float MaxZoomMultiplier = 5.f;

	/** Height above the ground (cm) at which the zoom reaches MaxZoomMultiplier. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float MaxZoomHeight = 1000.f;

	/** How quickly the zoom chases its height-based target. Higher = snappier. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.1))
	float ZoomInterpSpeed = 8.f;

	/** Safety net: the ability ends after this long even if the hero never lands. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.5))
	float MaxAirTime = 5.f;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) override;

private:
	/** One per-frame step while airborne: track height, update zoom, end on landing. */
	void StepAirborne();

	UPROPERTY()
	class AHeroCharacter* KickHero = nullptr;

	FTimerHandle AirTimer;

	float AirTime = 0.f;
	float LastStepTime = 0.f;
	float CurrentZoom = 1.f;
};
