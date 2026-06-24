// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ExpProDev.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/DefaultPlayerController.h"
#include "Weapon/Weapon.h"
#include "Gamemode/DefaultGameMode.h"
#include "TimerManager.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemPickup.h"
#include "HUD/PlayerHUD.h"
#include "Quest/QuestComponent.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 160.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SocketOffset = FVector(0.f, 70.f, 23.f);
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//Camera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "head");
	//Camera->SetupAttachment(GetMesh(), FName("head"));
	Camera->SetupAttachment(SpringArm);
	Camera->FieldOfView = 95.0f;
	Camera->bUsePawnControlRotation = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetRootComponent());

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	QuestComponent = CreateDefaultSubobject<UQuestComponent>(TEXT("QuestComponent"));

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	//TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//NetUpdateFrequency = 66.f;
	//MinNetUpdateFrequency = 33.f;
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(APlayerCharacter, OverlappingWeapon);
	DOREPLIFETIME_CONDITION(APlayerCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(APlayerCharacter, Health);
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->PlayerCharacter = this;
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();
	
	// If on server, and a proj bullet hits, it will recieve damage.
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &APlayerCharacter::RecieveDamage);
	}
	
	if (!HasAuthority())
	{
		PlayerController = PlayerController == nullptr ? Cast<ADefaultPlayerController>(Controller) : PlayerController;
		if (PlayerController)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(InputMapping, 0);
			}
		}
	}
}

// Required for Listen Servers.
void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	PlayerController = PlayerController == nullptr ? Cast<ADefaultPlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}
	
	UpdateHUDHealth();
}

void APlayerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	
	PlayerController = Cast<ADefaultPlayerController>(Controller);
	UpdateHUDHealth();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			UE_LOG(LogTemp, Display, TEXT("Binding MoveAction"));
			Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::MoveButton);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MoveAction is null!"));
		}
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::LookButton);
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::JumpButton);
		Input->BindAction(EquipAction, ETriggerEvent::Started, this, &APlayerCharacter::EquipButton);
		Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::CrouchButtonPressed);
		
		Input->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &APlayerCharacter::FireWeaponButtonPressed);
		Input->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &APlayerCharacter::FireWeaponButtonReleased);
		
		Input->BindAction(AimWeaponAction, ETriggerEvent::Started, this, &APlayerCharacter::AimWeaponButtonPressed);
		Input->BindAction(AimWeaponAction, ETriggerEvent::Completed, this, &APlayerCharacter::AimWeaponButtonReleased);

		if (InteractAction)
			Input->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::InteractButtonPressed);
		if (ScrollHotbarAction)
			Input->BindAction(ScrollHotbarAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ScrollHotbar);
		if (ToggleInventoryAction)
			Input->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleInventoryButtonPressed);
	}
}



void APlayerCharacter::MoveButton(const FInputActionInstance& Instance)
{
	FVector2D MovementDirection = Instance.GetValue().Get<FVector2D>();
	const FRotator Rotation(0.f, Controller->GetControlRotation().Yaw, Controller->GetControlRotation().Roll);
	const FVector RightDirection( FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y));
	const FVector ForwardDirection( FRotationMatrix(Rotation).GetUnitAxis(EAxis::X));
	AddMovementInput(RightDirection, MovementDirection.X);
	AddMovementInput(ForwardDirection, MovementDirection.Y);
}

void APlayerCharacter::LookButton(const FInputActionInstance& Instance)
{
	FVector2D LookDirection = Instance.GetValue().Get<FVector2D>();
	AddControllerYawInput(LookDirection.X);
	AddControllerPitchInput(LookDirection.Y);
}

void APlayerCharacter::JumpButton(const FInputActionInstance& Instance)
{
	Super::Jump();
	UE_LOG(LogTemp, Display, TEXT("JumpAction"));
}

void APlayerCharacter::EquipButton(const FInputActionInstance& Instance)
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else // We know it's from a client
		{
			ServerEquipButtonPressed();
		}
	}
	UE_LOG(LogTemp, Display, TEXT("EquipAction"));
}

void APlayerCharacter::ServerEquipButtonPressed_Implementation()
{
	// Don't need to check for authority as we know it'll be on the server.
	Combat->EquipWeapon(OverlappingWeapon);
}

void APlayerCharacter::CrouchButtonPressed(const FInputActionInstance& Instance)
{
	//
	// TODO: Implement Crouching.
	//
	
	Crouch();
}

void APlayerCharacter::FireWeaponButtonReleased(const FInputActionInstance& Instance)
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void APlayerCharacter::AimWeaponButtonPressed(const FInputActionInstance& Instance)
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void APlayerCharacter::AimWeaponButtonReleased(const FInputActionInstance& Instance)
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void APlayerCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr){return;}
	
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
/*
	if (Speed == 0.f && !bIsInAir) // Standing still + not jumping
	{
		FRotator CurrentAimRotation = FRotator(GetBaseAimRotation().Pitch, 0, 0);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Pitch;
		//bUseControllerRotationYaw = false;
	}
	if (Speed > 0.f && bIsInAir)
	{
		StartingAimRotation = FRotator(GetBaseAimRotation().Pitch, 0, 0);
		AO_Yaw = 0.f;
		//bUseControllerRotationYaw = true;
	}*/
	AO_Yaw = GetBaseAimRotation().Pitch;
}

void APlayerCharacter::FireWeaponButtonPressed(const FInputActionInstance& Instance)
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}


// This is bound to OnTakeAnyDamage
void APlayerCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	class AController* InstigatedController, class AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage,0.f, MaxHealth);

	//
	// TODO: RoseJ - Implement Hit React Montage
	// PlayHitReactMontage();
	//

	UpdateHUDHealth();
	
	if (Health <= 0.f)
	{
		ADefaultGameMode* DefaultGameMode = GetWorld()->GetAuthGameMode<ADefaultGameMode>();
		if (DefaultGameMode)
		{
			PlayerController = PlayerController == nullptr ? Cast<ADefaultPlayerController>(Controller) : PlayerController;
			ADefaultPlayerController* AttackerController = Cast<ADefaultPlayerController>(InstigatedController);
			
			DefaultGameMode->PlayerEliminated(this, PlayerController, AttackerController);
		}
	}
}

void APlayerCharacter::OnRep_Health()
{
	//
	// TODO: RoseJ - Implement Hit React Montage
	// PlayHitReactMontage();
	//

	UpdateHUDHealth();
}


void APlayerCharacter::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<ADefaultPlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDHealth(Health, MaxHealth);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to cast to Player Controller when updating HUD health"));
	}
}

void APlayerCharacter::AddXP(float Amount)
{
	if (Amount <= 0.f) return;

	XP += Amount;

	while (XP >= XPToNextLevel)
	{
		XP -= XPToNextLevel;
		Level++;
	}

	UpdateHUDXP();
}

void APlayerCharacter::UpdateHUDXP()
{
	PlayerController = PlayerController == nullptr ? Cast<ADefaultPlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDXP(XP, XPToNextLevel, Level);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to cast to Player Controller when updating HUD XP"));
	}
}

void APlayerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;

	// If function is called on the character that is being controlled...
	// This fixes the issue of the text not displaying on the server. 
	// (we already know we are on the server at this point)
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool APlayerCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool APlayerCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

//
// Eliminations
//

// Calling from the gamemode, just on the server alone.
void APlayerCharacter::Eliminated()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MulticastEliminated();
	GetWorldTimerManager().SetTimer(EliminatedTimer, this, &APlayerCharacter::EliminatedTimerFinished, EliminatedDelay);
}

void APlayerCharacter::MulticastEliminated_Implementation()
{
	bEliminated = true;

	// TODO: PlayElimMontage();
	// DEATH ANIMATIONS LEC:101
	// Dissolve effects LEC:103

	// Disable Player Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacter::InteractButtonPressed(const FInputActionInstance& Instance)
{
	if (!PendingPickup || !Inventory) return;

	if (Inventory->AddItem(PendingPickup->ItemDef, PendingPickup->Quantity))
	{
		AItemPickup* PickupToDestroy = PendingPickup;
		PendingPickup = nullptr;
		PickupToDestroy->Destroy();
	}
}

void APlayerCharacter::ToggleInventoryButtonPressed(const FInputActionInstance& Instance)
{
	APlayerHUD* HUD = PlayerController ? Cast<APlayerHUD>(PlayerController->GetHUD()) : nullptr;
	if (HUD) HUD->ToggleInventoryScreen();
}

void APlayerCharacter::ScrollHotbar(const FInputActionInstance& Instance)
{
	if (!Inventory) return;
	float ScrollValue = Instance.GetValue().Get<float>();
	int32 HotbarCount = Inventory->GetHotbarSlotCount();
	int32 Delta = ScrollValue > 0.f ? -1 : 1;
	int32 NewIndex = ((Inventory->ActiveHotbarIndex + Delta) % HotbarCount + HotbarCount) % HotbarCount;
	Inventory->SetActiveHotbarIndex(NewIndex);
}

void APlayerCharacter::SetPendingPickup(AItemPickup* Pickup)
{
	PendingPickup = Pickup;
}

void APlayerCharacter::ClearPendingPickupIfMatch(AItemPickup* Pickup)
{
	if (PendingPickup == Pickup)
		PendingPickup = nullptr;
}

// Will only be called on the server.
void APlayerCharacter::EliminatedTimerFinished()
{
	ADefaultGameMode* DefaultGameMode = GetWorld()->GetAuthGameMode<ADefaultGameMode>();
	if (DefaultGameMode)
	{
		// Make sure here to make sure the PlayerBP can Always Spawn even after adjusting location.
		DefaultGameMode->RequestRespawn(this, PlayerController); // Could just be Controller rather than the playercontroller
	}
}