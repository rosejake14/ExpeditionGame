// No Rights Reserved @ Team Expedition

#include "Hero/HeroCharacter.h"

#include "Abilities/HeroAttributeSet.h"
#include "Components/CombatComponent.h"
#include "ExpProDev.h"
#include "Hero/HeroDefinition.h"
#include "Weapon/Weapon.h"

#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"

void AHeroCharacter::ZoomCamera(float Multiplier, float Duration, float BlendTime)
{
	if (!ZoomArm) ZoomArm = FindComponentByClass<USpringArmComponent>();
	if (!ZoomArm || !GetWorld() || Duration <= 0.f)
	{
		UE_LOG(LogExpProDev, Warning, TEXT("HeroCharacter: ZoomCamera skipped (no spring arm or zero duration)."));
		return;
	}

	if (BaseArmLength < 0.f) BaseArmLength = ZoomArm->TargetArmLength;

	ZoomStartLength = ZoomArm->TargetArmLength;
	ZoomPeakLength  = BaseArmLength * Multiplier;
	ZoomReturnFrom  = ZoomPeakLength;
	ZoomDuration    = Duration;
	ZoomBlend       = FMath::Clamp(BlendTime, KINDA_SMALL_NUMBER, Duration * 0.5f);
	ZoomElapsed     = 0.f;
	ZoomLastTime    = GetWorld()->GetTimeSeconds();

	GetWorldTimerManager().ClearTimer(ZoomTimer);
	ZoomTimer = GetWorldTimerManager().SetTimerForNextTick(this, &AHeroCharacter::StepCameraZoom);
}

void AHeroCharacter::SetCameraZoom(float Multiplier)
{
	if (!ZoomArm) ZoomArm = FindComponentByClass<USpringArmComponent>();
	if (!ZoomArm) return;

	if (BaseArmLength < 0.f) BaseArmLength = ZoomArm->TargetArmLength;

	GetWorldTimerManager().ClearTimer(ZoomTimer);
	ZoomElapsed = 0.f;
	ZoomArm->TargetArmLength = BaseArmLength * Multiplier;
}

void AHeroCharacter::StopCameraZoom(float BlendTime)
{
	if (!ZoomArm || BaseArmLength < 0.f || !GetWorld()) return;

	// Jump straight into the return phase, starting from wherever the arm is now.
	ZoomReturnFrom = ZoomArm->TargetArmLength;
	ZoomBlend      = FMath::Max(BlendTime, KINDA_SMALL_NUMBER);
	ZoomDuration   = ZoomElapsed + ZoomBlend;
	ZoomLastTime   = GetWorld()->GetTimeSeconds();

	GetWorldTimerManager().ClearTimer(ZoomTimer);
	ZoomTimer = GetWorldTimerManager().SetTimerForNextTick(this, &AHeroCharacter::StepCameraZoom);
}

void AHeroCharacter::StepCameraZoom()
{
	if (!ZoomArm || !GetWorld()) return;

	const float Now = GetWorld()->GetTimeSeconds();
	ZoomElapsed += Now - ZoomLastTime;
	ZoomLastTime = Now;

	constexpr float EaseExponent = 2.f;

	if (ZoomElapsed >= ZoomDuration)
	{
		ZoomArm->TargetArmLength = BaseArmLength;
		return;
	}

	if (ZoomElapsed >= ZoomDuration - ZoomBlend)
	{
		// Return: ReturnFrom eases back down to the normal length.
		const float Alpha = (ZoomDuration - ZoomElapsed) / ZoomBlend;
		ZoomArm->TargetArmLength = FMath::InterpEaseInOut(BaseArmLength, ZoomReturnFrom, Alpha, EaseExponent);
	}
	else if (ZoomElapsed < ZoomBlend)
	{
		const float Alpha = ZoomElapsed / ZoomBlend;
		ZoomArm->TargetArmLength = FMath::InterpEaseInOut(ZoomStartLength, ZoomPeakLength, Alpha, EaseExponent);
	}
	else
	{
		ZoomArm->TargetArmLength = ZoomPeakLength;
	}

	ZoomTimer = GetWorldTimerManager().SetTimerForNextTick(this, &AHeroCharacter::StepCameraZoom);
}

void AHeroCharacter::SetHeroDefinition(UHeroDefinition* InDefinition)
{
	HeroDefinition = InDefinition;

	if (!HeroDefinition)
	{
		UE_LOG(LogExpProDev, Warning, TEXT("HeroCharacter: spawned with no hero definition."));
		return;
	}

	ApplyAppearance();

	UE_LOG(LogExpProDev, Log, TEXT("HeroCharacter: spawned as '%s'."), *HeroDefinition->HeroId.ToString());
}

void AHeroCharacter::ApplyAppearance()
{
	if (!HeroDefinition) return;

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		if (HeroDefinition->Mesh)
		{
			// SetSkeletalMeshAsset, not SetSkeletalMesh — the latter is deprecated on the
			// USkinnedMeshComponent base as of 5.1. Setting the anim class below re-inits the pose.
			MeshComponent->SetSkeletalMeshAsset(HeroDefinition->Mesh);
		}

		if (HeroDefinition->AnimClass)
		{
			MeshComponent->SetAnimInstanceClass(HeroDefinition->AnimClass);
		}

		if (HeroDefinition->OverrideMaterial)
		{
			// Every slot, so a mesh with several materials still reads as one hero.
			const int32 SlotCount = MeshComponent->GetNumMaterials();
			for (int32 Slot = 0; Slot < SlotCount; ++Slot)
			{
				MeshComponent->SetMaterial(Slot, HeroDefinition->OverrideMaterial);
			}
		}

		if (HeroDefinition->MeshScale > 0.f)
		{
			MeshComponent->SetRelativeScale3D(FVector(HeroDefinition->MeshScale));
		}
	}

	// Capsule half-height and radius are set together where supplied; a hero that changes one
	// without the other would either float above the ground or clip into it.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		const float HalfHeight = HeroDefinition->CapsuleHalfHeight > 0.f
			? HeroDefinition->CapsuleHalfHeight : Capsule->GetUnscaledCapsuleHalfHeight();
		const float Radius = HeroDefinition->CapsuleRadius > 0.f
			? HeroDefinition->CapsuleRadius : Capsule->GetUnscaledCapsuleRadius();

		if (HeroDefinition->CapsuleHalfHeight > 0.f || HeroDefinition->CapsuleRadius > 0.f)
		{
			Capsule->SetCapsuleSize(Radius, HalfHeight);
		}
	}
}

void AHeroCharacter::InitialiseAttributes()
{
	UHeroAttributeSet* HeroAttributes = GetAttributes();

	// No definition means something upstream failed — fall back to the inherited stat block
	// rather than spawning a hero with zeroed attributes.
	if (!HeroDefinition || !HeroAttributes)
	{
		Super::InitialiseAttributes();
		return;
	}

	// Movement speeds are plain members on APlayerCharacter, read by the sprint input handlers.
	WalkSpeed   = HeroDefinition->BaseWalkSpeed;
	SprintSpeed = HeroDefinition->BaseSprintSpeed;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = WalkSpeed;
	}

	HeroAttributes->InitMaxHealth(HeroDefinition->BaseMaxHealth);
	HeroAttributes->InitHealth(HeroDefinition->BaseMaxHealth);
	HeroAttributes->InitMaxStamina(HeroDefinition->BaseMaxStamina);
	HeroAttributes->InitStamina(HeroDefinition->BaseMaxStamina);
	HeroAttributes->InitDamageMultiplier(HeroDefinition->BaseDamageMultiplier);
	HeroAttributes->InitMoveSpeed(HeroDefinition->BaseWalkSpeed);
	HeroAttributes->InitMaxUltimateCharge(HeroDefinition->BaseMaxUltimateCharge);
	HeroAttributes->InitUltimateCharge(0.f);
	HeroAttributes->InitDamage(0.f);
}

void AHeroCharacter::PossessedBy(AController* NewController)
{
	// Super initialises the ability system and writes the starting attributes; abilities can
	// only be granted once that has happened.
	Super::PossessedBy(NewController);

	GrantAbilities();
	EquipStartingWeapon();
}

void AHeroCharacter::EquipStartingWeapon()
{
	UCombatComponent* CombatComponent = GetCombat();
	if (!HeroDefinition || !HeroDefinition->StartingWeapon || !CombatComponent) return;
	if (CombatComponent->GetEquippedWeapon()) return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(
		HeroDefinition->StartingWeapon, GetActorLocation(), GetActorRotation(), Params);
	if (!Weapon)
	{
		UE_LOG(LogExpProDev, Warning, TEXT("HeroCharacter: failed to spawn starting weapon."));
		return;
	}

	CombatComponent->EquipWeapon(Weapon);
	UE_LOG(LogExpProDev, Log, TEXT("HeroCharacter: equipped starting weapon '%s'."), *Weapon->GetName());
}

void AHeroCharacter::GrantAbilities()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC || !HeroDefinition) return;

	const TSubclassOf<UGameplayAbility> SlotAbilities[3] =
	{
		HeroDefinition->PowerAbility,
		HeroDefinition->AddonAbility,
		HeroDefinition->UltimateAbility
	};

	for (int32 SlotIndex = 0; SlotIndex < 3; ++SlotIndex)
	{
		// Clear a previous grant first, so re-possession can't stack duplicates of an ability.
		if (AbilityHandles[SlotIndex].IsValid())
		{
			ASC->ClearAbility(AbilityHandles[SlotIndex]);
			AbilityHandles[SlotIndex] = FGameplayAbilitySpecHandle();
		}

		if (!SlotAbilities[SlotIndex]) continue;

		FGameplayAbilitySpec Spec(SlotAbilities[SlotIndex], 1, INDEX_NONE, this);
		AbilityHandles[SlotIndex] = ASC->GiveAbility(Spec);

		// The instance exists as soon as GiveAbility returns (InstancedPerActor + ReplicateNo), and
		// GAS runs CanActivate/Commit on it, so this is where its cooldown tag gets decided.
		const FGameplayAbilitySpec* Granted = ASC->FindAbilitySpecFromHandle(AbilityHandles[SlotIndex]);
		if (UHeroAbility* Instance = Granted ? Cast<UHeroAbility>(Granted->GetPrimaryInstance()) : nullptr)
		{
			Instance->SetGrantedSlot(static_cast<EHeroAbilitySlot>(SlotIndex));
		}
	}
}

UHeroAbility* AHeroCharacter::GetAbilityForSlot(EHeroAbilitySlot InSlot) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	const int32 SlotIndex = static_cast<int32>(InSlot);
	if (!ASC || SlotIndex < 0 || SlotIndex >= 3 || !AbilityHandles[SlotIndex].IsValid())
	{
		return nullptr;
	}

	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(AbilityHandles[SlotIndex]);
	return Spec ? Cast<UHeroAbility>(Spec->Ability) : nullptr;
}

void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input) return;

	if (PowerAbilityAction)
		Input->BindAction(PowerAbilityAction, ETriggerEvent::Started, this, &AHeroCharacter::ActivatePower);
	if (AddonAbilityAction)
		Input->BindAction(AddonAbilityAction, ETriggerEvent::Started, this, &AHeroCharacter::ActivateAddon);
	if (UltimateAbilityAction)
		Input->BindAction(UltimateAbilityAction, ETriggerEvent::Started, this, &AHeroCharacter::ActivateUltimate);
}

void AHeroCharacter::ActivatePower()     { ActivateSlot(EHeroAbilitySlot::Power); }
void AHeroCharacter::ActivateAddon()     { ActivateSlot(EHeroAbilitySlot::Addon); }
void AHeroCharacter::ActivateUltimate()  { ActivateSlot(EHeroAbilitySlot::Ultimate); }

void AHeroCharacter::ActivateSlot(EHeroAbilitySlot InSlot)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	const int32 SlotIndex = static_cast<int32>(InSlot);
	if (!ASC || SlotIndex < 0 || SlotIndex >= 3) return;

	if (!AbilityHandles[SlotIndex].IsValid())
	{
		// Not an error — a hero simply may not have an ability in every slot.
		return;
	}

	// A failed activation is the normal "still cooling down" / "not charged" case, so this is
	// deliberately silent rather than a warning.
	ASC->TryActivateAbility(AbilityHandles[SlotIndex]);
}
