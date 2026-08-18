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
#include "Inventory/ItemDefinition.h"
#include "Upgrade/UpgradeManagerComponent.h"
#include "HUD/PlayerHUD.h"
#include "Quest/QuestManagerComponent.h"
#include "Interaction/Interactable.h"
#include "Save/ExpProSaveGame.h"
#include "Save/SaveGameSubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "Extraction/ExtractionTypes.h"
#include "Kismet/GameplayStatics.h"


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

	Inventory       = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	QuestManager    = CreateDefaultSubobject<UQuestManagerComponent>(TEXT("QuestManager"));
	UpgradeManager  = CreateDefaultSubobject<UUpgradeManagerComponent>(TEXT("UpgradeManager"));

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
		Combat->PlayerCharacter = this;

	BaseMaxHealth = MaxHealth;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	UpdateHUDHealth();

	// TECH_DEBT(TD-BUG-14): 0.2s one-shot timer is a race workaround — the HUD widgets aren't
	// constructed yet at pawn BeginPlay, so the XP push is simply delayed and hoped to land.
	FTimerHandle XPHUDTimer;
	GetWorldTimerManager().SetTimer(XPHUDTimer, this, &APlayerCharacter::UpdateHUDXP, 0.2f, false);

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

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Force any deferred (coalesced) save state to disk before we tear down — an async flush
	// scheduled on the timer could otherwise be dropped when the game exits.
	USaveGameSubsystem::FlushNow(this);
	Super::EndPlay(EndPlayReason);
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
		if (ExtractAction)
			Input->BindAction(ExtractAction, ETriggerEvent::Started, this, &APlayerCharacter::ExtractButtonPressed);
		if (ScrollHotbarAction)
			Input->BindAction(ScrollHotbarAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ScrollHotbar);
		if (ToggleInventoryAction)
			Input->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleInventoryButtonPressed);
		if (SprintAction)
		{
			Input->BindAction(SprintAction, ETriggerEvent::Started,   this, &APlayerCharacter::SprintButtonPressed);
			Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::SprintButtonReleased);
		}
	}
}



void APlayerCharacter::MoveButton(const FInputActionInstance& Instance)
{
	FVector2D MovementDirection = Instance.GetValue().Get<FVector2D>();
	// TECH_DEBT(TD-BUG-8): Controller is dereferenced unguarded — input can still be routed for a
	// frame after unpossession (e.g. during the elimination/respawn handoff).
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
	// TECH_DEBT(TD-STUB-1): crouch is bound and bIsCrouched reaches the anim BP, but no crouch
	// capsule sizing, speed change or animation set exists — pressing it does nothing visible.
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

// TECH_DEBT(TD-ARCH-18): this is the only reason the pawn ticks every frame, and all it does is
// copy one rotation value the anim instance could read itself. The turn-in-place work below was
// abandoned half-finished, and AO_Yaw actually holds Pitch — the name is a leftover.
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
	if (bEliminated) return;

	Health = FMath::Clamp(Health - Damage,0.f, MaxHealth);

	// TECH_DEBT(TD-STUB-2): hit reactions were never implemented — taking damage has no animation
	// or camera feedback anywhere in the game. PlayHitReactMontage() does not exist.
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
	// TECH_DEBT(TD-STUB-2): see RecieveDamage — the client-side hit reaction is stubbed too.
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

	const int32 OldLevel = Level;
	while (XP >= GetXPToNextLevel())
	{
		XP -= GetXPToNextLevel();
		Level++;
	}

	UpdateHUDXP();

	const bool bLeveledUp = (Level != OldLevel);
	if (bLeveledUp && LevelUpSound)
		UGameplayStatics::PlaySound2D(this, LevelUpSound);

	// Level-up is a checkpoint (flush now); a plain XP tick is deferred (coalesced).
	SavePlayerData(/*bDeferred*/ !bLeveledUp);
}

void APlayerCharacter::UpdateHUDXP()
{
	PlayerController = PlayerController == nullptr ? Cast<ADefaultPlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDXP(XP, GetXPToNextLevel(), Level);
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
	// Mark eliminated + disable collision FIRST, so the pickups we spawn next don't overlap this
	// dying pawn and get grabbed straight back during the respawn delay.
	MulticastEliminated();
	// Scatter everything the player was carrying at the death location before they respawn.
	DropAllItemsOnDeath();
	GetWorldTimerManager().SetTimer(EliminatedTimer, this, &APlayerCharacter::EliminatedTimerFinished, EliminatedDelay);
}

void APlayerCharacter::DropAllItemsOnDeath()
{
	if (!Inventory) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Origin = GetActorLocation();

	for (int32 i = 0; i < Inventory->GetTotalSlotCount(); ++i)
	{
		const FInventorySlot& Slot = Inventory->GetSlot(i);
		if (Slot.IsEmpty() || !Slot.ItemDef) continue;

		TSubclassOf<AItemPickup> PickupClass =
			Slot.ItemDef->PickupClass ? Slot.ItemDef->PickupClass : DefaultDropPickupClass;
		if (!PickupClass)
		{
			// No pickup class configured — leave the item in the inventory rather than destroying it.
			UE_LOG(LogTemp, Warning,
				TEXT("DropAllItemsOnDeath: '%s' has no PickupClass and no DefaultDropPickupClass fallback — not dropped."),
				*Slot.ItemDef->GetName());
			continue;
		}

		// Random point within the scatter radius, then dropped onto the floor beneath it.
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Dist  = FMath::FRandRange(0.f, DeathDropScatterRadius);
		const FVector Near = Origin + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
		const FVector SpawnLocation = AItemPickup::GroundedLocation(World, Near, this);

		FActorSpawnParameters SpawnParams;
		// AlwaysSpawn (not AdjustIfPossible) so clustered drops land exactly on the traced floor
		// point instead of being shoved upward to resolve overlap with each other.
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AItemPickup* Pickup = World->SpawnActor<AItemPickup>(
			PickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
		{
			Pickup->ItemDef  = Slot.ItemDef;
			Pickup->Quantity = Slot.Quantity;

			// Only clear the slot once its pickup is safely in the world.
			// TECH_DEBT(TD-BUG-21): Slot aliases the live array element that RemoveItem clears, and
			// SpawnActor above can run overlap callbacks that also mutate the inventory. Safe today
			// only because dead players are filtered out of pickup — copy the slot out first.
			Inventory->RemoveItem(i, Slot.Quantity);
		}
	}
}

void APlayerCharacter::MulticastEliminated_Implementation()
{
	bEliminated = true;

	// TECH_DEBT(TD-STUB-3): death animation and dissolve effects were never implemented — the pawn
	// just freezes in place for EliminatedDelay seconds, then vanishes.

	// Disable Player Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	// TECH_DEBT(TD-BUG-20): PlayerController is only populated on paths that ran the lazy cast
	// (BeginPlay / PossessedBy / OnRep_Controller). If none did, input is silently left enabled.
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
	if (!PendingInteractable) return;
	if (IInteractable* I = Cast<IInteractable>(PendingInteractable))
		I->Interact(this);
}

void APlayerCharacter::ExtractButtonPressed(const FInputActionInstance& Instance)
{
	// Only extracts when standing in an extraction zone; separate key from generic Interact.
	if (!CurrentExtractionZone) return;

	// Drop the on-screen prompt now that the sell summary is taking over.
	if (APlayerHUD* HUD = GetPlayerHUD())
		HUD->HideExtractPrompt();

	// The freeze is applied when the sell summary is shown (and released by its Continue button),
	// so it stays balanced even if the summary can't be displayed.
	if (IInteractable* Zone = Cast<IInteractable>(CurrentExtractionZone))
		Zone->Interact(this);
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

void APlayerCharacter::SetPendingInteractable(AActor* Interactable)
{
	PendingInteractable = Interactable;
}

void APlayerCharacter::ClearPendingInteractableIfMatch(AActor* Interactable)
{
	if (PendingInteractable == Interactable)
		PendingInteractable = nullptr;
}

void APlayerCharacter::SetExtractionZone(AActor* Zone)
{
	CurrentExtractionZone = Zone;

	if (APlayerHUD* HUD = GetPlayerHUD())
		HUD->ShowExtractPrompt(BuildExtractPromptText());
}

void APlayerCharacter::ClearExtractionZoneIfMatch(AActor* Zone)
{
	if (CurrentExtractionZone == Zone)
	{
		CurrentExtractionZone = nullptr;
		if (APlayerHUD* HUD = GetPlayerHUD())
			HUD->HideExtractPrompt();
	}
}

APlayerHUD* APlayerCharacter::GetPlayerHUD() const
{
	if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(Controller))
		return Cast<APlayerHUD>(PC->GetHUD());
	return nullptr;
}

FText APlayerCharacter::BuildExtractPromptText() const
{
	FText KeyName = FText::FromString(TEXT("Extract"));

	if (ExtractAction)
	{
		if (const APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				const TArray<FKey> Keys = Subsystem->QueryKeysMappedToAction(ExtractAction);
				if (Keys.Num() > 0)
					KeyName = Keys[0].GetDisplayName();
			}
		}
	}

	return FText::Format(
		NSLOCTEXT("Extraction", "ExtractPrompt", "Press [{0}] to Extract"), KeyName);
}

void APlayerCharacter::FreezeForExtraction()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
}

void APlayerCharacter::UnfreezeAfterExtraction()
{
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Bring the prompt back if the player is still standing in the extraction zone.
	if (CurrentExtractionZone)
	{
		if (APlayerHUD* HUD = GetPlayerHUD())
			HUD->ShowExtractPrompt(BuildExtractPromptText());
	}
}

void APlayerCharacter::SprintButtonPressed(const FInputActionInstance& Instance)
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APlayerCharacter::SprintButtonReleased(const FInputActionInstance& Instance)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APlayerCharacter::WipeSave()
{
	XP       = 0.f;
	Level    = 1;
	DOSCoins = 0;
	DamageMultiplier = 1.0f;
	MaxHealth = BaseMaxHealth;
	Health    = MaxHealth;

	if (UpgradeManager)
		UpgradeManager->ClearUpgrades();

	if (USaveGameSubsystem* Sub = GetGameInstance()->GetSubsystem<USaveGameSubsystem>())
		UGameplayStatics::DeleteGameInSlot(Sub->GetActiveSlotName(), 0);
	// Drop the in-memory cache too, or a queued flush would re-create the file we just deleted.
	USaveGameSubsystem::DiscardCache(this);
	UpdateHUDXP();
	UE_LOG(LogTemp, Warning, TEXT("WipeSave: save deleted, all stats and upgrades reset."));
}

void APlayerCharacter::SetLevel(int32 NewLevel)
{
	if (NewLevel < 1) return;
	Level = NewLevel;
	XP    = 0.f;
	SavePlayerData();
	UpdateHUDXP();
	UE_LOG(LogTemp, Warning, TEXT("SetLevel: player level set to %d."), NewLevel);
}

void APlayerCharacter::SetDOSCoins(int32 Amount)
{
	if (Amount < 0) return;
	DOSCoins = Amount;
	SavePlayerData();
	UE_LOG(LogTemp, Warning, TEXT("SetDOSCoins: DOS$ set to %d."), Amount);
}

void APlayerCharacter::SavePlayerData(bool bDeferred)
{
	// Read-modify-write through the subsystem: fields we don't touch here (e.g. PurchasedWeapons)
	// are preserved, so saving XP/coins can never wipe shop purchases. Deferred writes (coin/XP
	// ticks) are coalesced by the subsystem; checkpoints flush immediately.
	USaveGameSubsystem::MutateActiveSlot(this, [this](UExpProSaveGame& Save)
	{
		Save.XP       = XP;
		Save.Level    = Level;
		Save.DOSCoins = DOSCoins;
		if (UpgradeManager)
			Save.PurchasedUpgrades = UpgradeManager->GetAllPurchases();
	}, bDeferred ? ESaveFlushPolicy::Deferred : ESaveFlushPolicy::Checkpoint);
}

void APlayerCharacter::LoadPlayerData()
{
	UExpProSaveGame* Save = USaveGameSubsystem::LoadActiveSlot(this);
	if (!Save) return;

	XP       = Save->XP;
	Level    = Save->Level;
	DOSCoins = Save->DOSCoins;

	if (UpgradeManager)
	{
		UpgradeManager->LoadUpgrades(Save->PurchasedUpgrades);
		MaxHealth        = BaseMaxHealth + UpgradeManager->GetHealthBonus();
		Health           = MaxHealth;
		DamageMultiplier = 1.0f + UpgradeManager->GetDamageMultiplierBonus();
	}
}

void APlayerCharacter::AddDOSCoins(int32 Amount)
{
	if (Amount <= 0) return;
	DOSCoins += Amount;
	// High-frequency (coin pickups) — coalesce the write instead of hitching per coin.
	SavePlayerData(/*bDeferred*/ true);
}

void APlayerCharacter::SellLoot()
{
	if (!Inventory) return;

	// Valuation, inventory removal and coin credit all live in the economy subsystem now;
	// the pawn just kicks it off and drives the HUD from the returned totals.
	UEconomySubsystem* Economy = UEconomySubsystem::Get(this);
	if (!Economy) return;

	const FSellResult Result = Economy->SellLoot(Inventory);

	// Keep the pawn's cached balance in sync with the save the subsystem just credited.
	DOSCoins = Result.NewBalance;

	PlayerController = PlayerController == nullptr ? Cast<ADefaultPlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		if (APlayerHUD* HUD = Cast<APlayerHUD>(PlayerController->GetHUD()))
		{
			HUD->ShowSellSummary(Result.Entries, Result.TotalEarned, DOSCoins);
		}
	}
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