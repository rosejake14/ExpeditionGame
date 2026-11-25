// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HeroAbility.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/Actor.h"
#include "MolotovAbilities.generated.h"

//
//  Molotov Guy's kit. Spec: doc/hero/HERO_MOLOTOV_ABILITIES.md.
//  Main (M1) is undecided (whatever StartingWeapon is), Addon (F) is empty for now.
//

class AMolotovFire;

/** Everything a fire patch needs, handed from the ability through the projectile to the fire. */
struct FMolotovFireSettings
{
	float Radius = 500.f;
	float Duration = 6.f;
	float TickDamage = 5.f;     // already scaled by the thrower's DamageMultiplier
	float TickInterval = 0.5f;
	float Height = 200.f;       // how far above the ground point a pawn can be and still burn
};

/**
 *  Power (E). Throws a molotov from just in front of the mesh toward the crosshair, in a gravity
 *  arc. It breaks on anything: a pawn it hits directly takes ImpactDamage, and a fire patch
 *  always spawns on the ground below the impact point.
 */
UCLASS()
class EXPPRODEV_API UHeroAbility_ThrowMolotov : public UHeroAbility
{
	GENERATED_BODY()
public:
	UHeroAbility_ThrowMolotov();

	/** Launch speed, cm/s. Sets the arc's range. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float ThrowSpeed = 1500.f;

	/** Direct-hit damage, before the hero's DamageMultiplier. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float ImpactDamage = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float FireRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.1))
	float FireDuration = 6.f;

	/** Damage per tick, before the hero's DamageMultiplier. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float FireTickDamage = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.05))
	float FireTickInterval = 0.5f;

	/** How far in front of the mesh centre the molotov spawns, cm. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float SpawnForwardOffset = 60.f;

	/** How far above the fire's ground point a pawn can be and still burn, cm. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float FireHeight = 200.f;

	/** Projectile to throw. Point at a BP child for custom visuals. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability")
	TSubclassOf<class AMolotovProjectile> MolotovClass;

	/** Fire patch to spawn. Point at a BP child to add Niagara in OnFireStarted. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability")
	TSubclassOf<AMolotovFire> FireClass;

protected:
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) override;
};

/**
 *  The thrown bottle: a small sphere on a gravity arc. Breaks on the first thing it hits —
 *  damages that thing if it's a pawn other than the thrower, then drops a fire on the ground
 *  below the impact point and destroys itself.
 *
 *  Standalone rather than an AProjectile child: AProjectile ignores the Pawn channel (can't hit
 *  enemy capsules) and its OnHit carries TD-DEAD-4 / TD-STUB-2.
 */
UCLASS()
class EXPPRODEV_API AMolotovProjectile : public AActor
{
	GENERATED_BODY()
public:
	AMolotovProjectile();

	/** Call between SpawnActorDeferred and FinishSpawning, so the speed is set before movement initialises. */
	void InitMolotov(float InSpeed, float InImpactDamage, TSubclassOf<AMolotovFire> InFireClass,
		const FMolotovFireSettings& InFireSettings);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* Movement;

	UPROPERTY()
	TSubclassOf<AMolotovFire> FireClass;

	float ImpactDamage = 0.f;
	FMolotovFireSettings FireSettings;

	/** Guards against a second hit event in the same frame spawning a second fire. */
	bool bBroken = false;
};

/**
 *  A burning patch on the ground. Every TickInterval it damages each pawn inside Radius (and no
 *  more than Height above the ground point), except the thrower. No afterburn: damage only lands
 *  on ticks while a pawn is inside. Overlapping fires are separate actors, so they stack.
 *
 *  Draws an orange debug circle while burning (compiled out of Shipping). Add real visuals in a
 *  BP child via OnFireStarted.
 */
UCLASS(Blueprintable)
class EXPPRODEV_API AMolotovFire : public AActor
{
	GENERATED_BODY()
public:
	AMolotovFire();

	/** Call between SpawnActorDeferred and FinishSpawning. */
	void InitFire(const FMolotovFireSettings& InSettings) { Settings = InSettings; }

	/** BP presentation hook: spawn the Niagara fire here. Fired once, in BeginPlay. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Molotov")
	void OnFireStarted(float Radius, float Duration);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void TickFire();

	FMolotovFireSettings Settings;
	FTimerHandle TickTimer;
};

/**
 *  Ultimate (Q). Breathes a sustained cone of fire along the camera direction for BurpDuration.
 *  Every BurpTickInterval each pawn in the cone (except the hero) takes BurpTickDamage. Passes
 *  through walls. The hero is slowed to MoveSpeedMultiplier while burping. Can't be ended early.
 *
 *  Presentation: start a Niagara stream in OnAbilityActivated, stop it in On End Ability.
 */
UCLASS()
class EXPPRODEV_API UHeroAbility_GunpowderBurp : public UHeroAbility
{
	GENERATED_BODY()
public:
	UHeroAbility_GunpowderBurp();

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.1))
	float BurpDuration = 5.f;

	/** Damage per tick, before the hero's DamageMultiplier. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float BurpTickDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.05))
	float BurpTickInterval = 0.2f;

	/** Reach of the cone, cm. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0))
	float ConeLength = 1500.f;

	/** Full cone angle, degrees. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 1.0, ClampMax = 179.0))
	float ConeAngle = 60.f;

	/** Walk speed multiplier while burping. */
	UPROPERTY(EditDefaultsOnly, Category = "Hero Ability", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float MoveSpeedMultiplier = 0.5f;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) override;

private:
	void BurpTick();
	void OnBurpFinished();

	UPROPERTY()
	class AHeroCharacter* BurpHero = nullptr;

	FTimerHandle TickTimer;
	FTimerHandle EndTimer;

	/** MaxWalkSpeed when the burp started. Restored in EndAbility. */
	float SavedMaxWalkSpeed = 0.f;
};
