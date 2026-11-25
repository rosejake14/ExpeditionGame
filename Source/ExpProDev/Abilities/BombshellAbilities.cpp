// No Rights Reserved @ Team Expedition

#include "Abilities/BombshellAbilities.h"

#include "Components/CombatComponent.h"
#include "ExpProDev.h"
#include "Hero/HeroCharacter.h"
#include "Weapon/Weapon.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// ── Spin Shoot (Power) ────────────────────────────────────────────────────

UHeroAbility_SpinShoot::UHeroAbility_SpinShoot()
{
	CooldownSeconds = 15.f;
	bEndsImmediately = false;
	DisplayName = NSLOCTEXT("HeroAbilities", "SpinShoot", "Spin Shoot");
}

bool UHeroAbility_SpinShoot::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AHeroCharacter* Hero = GetHeroCharacter(ActorInfo);
	const UCombatComponent* CombatComponent = Hero ? Hero->GetCombat() : nullptr;
	if (!CombatComponent || !CombatComponent->GetEquippedWeapon())
	{
		UE_LOG(LogExpProDev, Warning, TEXT("SpinShoot: no weapon equipped, not activating."));
		return false;
	}
	return true;
}

void UHeroAbility_SpinShoot::ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo)
{
	AHeroCharacter* Hero = GetHeroCharacter(ActorInfo);
	if (!Hero || !Hero->GetWorld())
	{
		FinishAbility(true);
		return;
	}

	SpinningHero = Hero;

	// Spin the mesh only, never the actor. Rotating the actor swings the mesh (and the spring arm
	// attached to it) around the capsule centre, which shook the camera.
	if (USkeletalMeshComponent* Mesh = Hero->GetMesh())
	{
		SavedMeshRotation = Mesh->GetRelativeRotation();
	}

	// The spring arm is a child of the mesh, so the spinning mesh would drag the camera with it.
	// Move the arm onto the capsule (which doesn't spin) for the duration, keeping its world
	// position, and put it back in EndAbility.
	SpinArm = Hero->FindComponentByClass<USpringArmComponent>();
	if (SpinArm)
	{
		SavedArmParent = SpinArm->GetAttachParent();
		SavedArmSocket = SpinArm->GetAttachSocketName();
		SavedArmRelative = SpinArm->GetRelativeTransform();
		SpinArm->AttachToComponent(Hero->GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		UE_LOG(LogExpProDev, Warning, TEXT("SpinShoot: no spring arm on the hero, camera re-parent skipped."));
	}

	if (ZoomMultiplier > 1.f)
	{
		Hero->ZoomCamera(ZoomMultiplier, SpinCount * 360.f / SpinSpeed, ZoomBlendTime);
	}

	Elapsed = 0.f;
	ShotAccumulator = 0.f;
	LastStepTime = Hero->GetWorld()->GetTimeSeconds();

	// Step every frame rather than at a fixed rate, so the spin can't judder against the frame rate.
	SpinTimer = Hero->GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UHeroAbility_SpinShoot::StepSpin);

	UE_LOG(LogExpProDev, Log, TEXT("SpinShoot: started (%d turns at %.0f deg/s, %.1f shots/s)."),
		SpinCount, SpinSpeed, ShotsPerSecond);
}

void UHeroAbility_SpinShoot::StepSpin()
{
	UWorld* World = SpinningHero ? SpinningHero->GetWorld() : nullptr;
	if (!World)
	{
		FinishAbility(true);
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float Duration = SpinCount * 360.f / SpinSpeed;

	// Clamp the last step so the spin never overshoots its turn count.
	const float StepTime = FMath::Min(Now - LastStepTime, Duration - Elapsed);
	LastStepTime = Now;
	Elapsed += StepTime;

	if (USkeletalMeshComponent* Mesh = SpinningHero->GetMesh())
	{
		Mesh->SetRelativeRotation(SavedMeshRotation + FRotator(0.f, SpinSpeed * Elapsed, 0.f));
	}

	const float ShotInterval = 1.f / ShotsPerSecond;
	ShotAccumulator += StepTime;
	while (ShotAccumulator >= ShotInterval)
	{
		ShotAccumulator -= ShotInterval;
		FireShot();
	}

	if (Elapsed >= Duration)
	{
		FinishAbility();
		return;
	}

	SpinTimer = World->GetTimerManager().SetTimerForNextTick(this, &UHeroAbility_SpinShoot::StepSpin);
}

void UHeroAbility_SpinShoot::FireShot()
{
	UCombatComponent* CombatComponent = SpinningHero ? SpinningHero->GetCombat() : nullptr;
	AWeapon* Weapon = CombatComponent ? CombatComponent->GetEquippedWeapon() : nullptr;
	if (!Weapon) return;

	// Flat ring: actor facing plus the spin so far. Computed from yaw rather than the mesh's own
	// forward, because skeletal meshes usually sit at a -90 yaw offset to the actor.
	const float ShotYaw = SpinningHero->GetActorRotation().Yaw + SpinSpeed * Elapsed;
	const FVector Start = Weapon->GetActorLocation();
	Weapon->Fire(Start + FRotator(0.f, ShotYaw, 0.f).Vector() * ShotRange);
}

void UHeroAbility_SpinShoot::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Runs on normal finish and on cancel (e.g. death), so rotation is always handed back.
	if (SpinningHero)
	{
		if (UWorld* World = SpinningHero->GetWorld())
		{
			World->GetTimerManager().ClearTimer(SpinTimer);
		}

		if (USkeletalMeshComponent* Mesh = SpinningHero->GetMesh())
		{
			Mesh->SetRelativeRotation(SavedMeshRotation);
		}

		// A normal finish lets the shared zoom run out on its own; a cancel eases back now.
		if (bWasCancelled)
		{
			SpinningHero->StopCameraZoom(ZoomBlendTime);
		}

		if (SpinArm)
		{
			if (SavedArmParent)
			{
				SpinArm->AttachToComponent(SavedArmParent, FAttachmentTransformRules::KeepRelativeTransform, SavedArmSocket);
				SpinArm->SetRelativeTransform(SavedArmRelative);
			}
			SpinArm = nullptr;
		}

		UE_LOG(LogExpProDev, Log, TEXT("SpinShoot: ended%s."), bWasCancelled ? TEXT(" (cancelled)") : TEXT(""));
		SpinningHero = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ── Bomb Kick (Ultimate) ──────────────────────────────────────────────────

UHeroAbility_BombKick::UHeroAbility_BombKick()
{
	bChargeGated = true;
	bEndsImmediately = false; // stays active until landing, for the height-based zoom
	DisplayName = NSLOCTEXT("HeroAbilities", "BombKick", "Bomb Kick");
}

void UHeroAbility_BombKick::ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo)
{
	AHeroCharacter* Hero = GetHeroCharacter(ActorInfo);
	if (!Hero || !Hero->GetWorld())
	{
		FinishAbility(true);
		return;
	}

	// Ignore the caster so the bomb can't hurt its own hero.
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Hero);

	const float Damage = GetScaledDamage(BlastDamage);
	UGameplayStatics::ApplyRadialDamage(
		Hero,
		Damage,
		Hero->GetActorLocation(),
		BlastRadius,
		nullptr,
		IgnoreActors,
		Hero,
		Hero->GetController(),
		/*bDoFullDamage*/ true);

	// bXYOverride false keeps horizontal momentum; bZOverride true gives a consistent height.
	Hero->LaunchCharacter(FVector(0.f, 0.f, LaunchStrength), false, true);

	UE_LOG(LogExpProDev, Log, TEXT("BombKick: fired (%.0f damage, %.0f radius, %.0f launch)."),
		Damage, BlastRadius, LaunchStrength);

	KickHero = Hero;
	AirTime = 0.f;
	CurrentZoom = 1.f;
	LastStepTime = Hero->GetWorld()->GetTimeSeconds();
	AirTimer = Hero->GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UHeroAbility_BombKick::StepAirborne);
}

void UHeroAbility_BombKick::StepAirborne()
{
	UWorld* World = KickHero ? KickHero->GetWorld() : nullptr;
	if (!World)
	{
		FinishAbility(true);
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float DeltaTime = Now - LastStepTime;
	LastStepTime = Now;
	AirTime += DeltaTime;

	// Height of the feet above whatever is directly below. Nothing below within range counts as max.
	const float HalfHeight = KickHero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Feet = KickHero->GetActorLocation() - FVector(0.f, 0.f, HalfHeight);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BombKickGround), false, KickHero);
	const bool bHit = World->LineTraceSingleByChannel(
		Hit, Feet, Feet - FVector(0.f, 0.f, MaxZoomHeight), ECC_Visibility, Params);
	const float Height = bHit ? Feet.Z - Hit.ImpactPoint.Z : MaxZoomHeight;

	const float TargetZoom = FMath::Lerp(1.f, MaxZoomMultiplier, FMath::Clamp(Height / MaxZoomHeight, 0.f, 1.f));
	CurrentZoom = FMath::FInterpTo(CurrentZoom, TargetZoom, DeltaTime, ZoomInterpSpeed);
	KickHero->SetCameraZoom(CurrentZoom);

	// Short grace period so the launch has lifted off before "on the ground" can end it.
	constexpr float LiftOffGrace = 0.2f;
	const UCharacterMovementComponent* Movement = KickHero->GetCharacterMovement();
	const bool bLanded = AirTime > LiftOffGrace && Movement && Movement->IsMovingOnGround();

	if (bLanded || AirTime >= MaxAirTime)
	{
		FinishAbility(!bLanded);
		return;
	}

	AirTimer = World->GetTimerManager().SetTimerForNextTick(this, &UHeroAbility_BombKick::StepAirborne);
}

void UHeroAbility_BombKick::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Runs on landing, timeout and cancel (e.g. death), so the camera always comes back.
	if (KickHero)
	{
		if (UWorld* World = KickHero->GetWorld())
		{
			World->GetTimerManager().ClearTimer(AirTimer);
		}

		constexpr float ZoomReturnTime = 0.3f;
		KickHero->StopCameraZoom(ZoomReturnTime);

		UE_LOG(LogExpProDev, Log, TEXT("BombKick: ended after %.2fs%s."), AirTime,
			bWasCancelled ? TEXT(" (cancelled / timed out)") : TEXT(" (landed)"));
		KickHero = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
