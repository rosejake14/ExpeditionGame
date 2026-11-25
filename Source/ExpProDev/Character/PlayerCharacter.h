// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"

#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UHeroAttributeSet;



// TECH_DEBT(TD-ARCH-1): god object — this one class owns input, movement, sprint, health,
// elimination, XP/levelling, currency, save/load orchestration, selling, interaction dispatch,
// extraction-zone state and direct HUD pushes. Every gameplay feature ends up editing it.
//
// TECH_DEBT(TD-ARCH-8): the replication scaffolding below (OverlappingWeapon,
// Server/NetMulticast RPCs, HasAuthority branching) is inherited from the multiplayer prototype.
// The Health OnRep that used to be part of this was removed when health moved into
// UHeroAttributeSet — GAS owns attribute replication now.
// The project is singleplayer-first, so none of it is exercised or tested, but it still shapes
// how every new feature has to be written.
UCLASS()
class EXPPRODEV_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* InputMapping;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* FireWeaponAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* AimWeaponAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* InteractAction;

	// Dedicated "extract / sell loot" key, separate from the generic Interact key.
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ExtractAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ScrollHotbarAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ToggleInventoryAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	float WalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	float SprintSpeed = 1000.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Flush any deferred save state synchronously on quit / level change.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called when possed, parsing issues when clients connecting too early - before BeingPlay(), meaning they had no controller
	virtual void PossessedBy(AController* NewController) override;

	void OnRep_Controller() override;

	void MoveButton(const FInputActionInstance& Instance);
	void LookButton(const FInputActionInstance& Instance);
	void JumpButton(const FInputActionInstance& Instance);
	void EquipButton(const FInputActionInstance& Instance);
	void CrouchButtonPressed(const FInputActionInstance& Instance);
	void FireWeaponButtonPressed(const FInputActionInstance& Instance);
	void FireWeaponButtonReleased(const FInputActionInstance& Instance);
	void AimWeaponButtonPressed(const FInputActionInstance& Instance);
	void AimWeaponButtonReleased(const FInputActionInstance& Instance);
	void InteractButtonPressed(const FInputActionInstance& Instance);
	void ExtractButtonPressed(const FInputActionInstance& Instance);
	void ScrollHotbar(const FInputActionInstance& Instance);
	void ToggleInventoryButtonPressed(const FInputActionInstance& Instance);
	void SprintButtonPressed(const FInputActionInstance& Instance);
	void SprintButtonReleased(const FInputActionInstance& Instance);
	void AimOffset(float DeltaTime);
	UFUNCTION() // Callback events ALWAYS need to be UFUNCTIONS
	void RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatedController, class AActor* DamageCauser);
	void UpdateHUDHealth();

	/**
	 *  Fired by the ability system whenever Health or MaxHealth changes, from any source —
	 *  damage, a heal, an upgrade effect. Bound in PossessedBy so the HUD can never fall out of
	 *  step with the attributes the way it did when every caller had to remember to push it.
	 *  Not a UFUNCTION: FOnGameplayAttributeValueChange is a plain multicast delegate.
	 */
	void HandleHealthAttributeChanged(const struct FOnAttributeChangeData& Data);

	//	Blueprint Editable Variables

	UPROPERTY(EditAnywhere, Category = "Audio")
	class USoundBase* LevelUpSound;

	// Player Health:
	//
	// Health and MaxHealth now live in UHeroAttributeSet — this is the designer-facing starting
	// value they are initialised from, and what purchased health upgrades are added on top of.
	// If BP_PlayerCharacter overrode the old MaxHealth property, set the same number here.
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float BaseMaxHealth = 100.f;

	/**
	 *  Writes BaseMaxHealth and the other starting stats into the attribute set.
	 *  AHeroCharacter overrides this to take its numbers from its UHeroDefinition instead.
	 */
	virtual void InitialiseAttributes();

	/** The attribute set, for subclasses that need to write starting values into it. */
	class UHeroAttributeSet* GetAttributes() const { return Attributes; }

	// Player XP / Level:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerStats)
	int32 Level = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float XP = 0.f;
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float BaseXP = 100.f;
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float XPPerLevel = 10.f;
	void UpdateHUDXP();

	// DOSCoin currency (persists across runs via save):
	UPROPERTY(BlueprintReadOnly, Category = PlayerStats)
	int32 DOSCoins = 0;

	// Player Eliminated:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	bool bEliminated = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	FTimerHandle EliminatedTimer;
	void EliminatedTimerFinished();
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerStats)
	float EliminatedDelay = 2.f;

	// ── Death drops ───────────────────────────────────────────────────────
	// Radius (uu) around the death spot that carried items are scattered within.
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathDropScatterRadius = 300.f;

	// Fallback pickup actor for items whose UItemDefinition has no PickupClass set.
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TSubclassOf<class AItemPickup> DefaultDropPickupClass;

	// Spawns every carried inventory item as a scattered pickup, then empties the inventory.
	void DropAllItemsOnDeath();


private:
	// Player Character Components
	//UPROPERTY(VisibleAnywhere, Category = Camera)
	//class USpringArmComponent* SpringArm;
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* Camera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* SpringArm;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UAbilitySystemComponent* AbilitySystemComponent;

	// Not a component — a plain subobject. UAbilitySystemComponent::InitializeComponent scans the
	// owner for UAttributeSet subobjects and registers this one automatically.
	UPROPERTY()
	class UHeroAttributeSet* Attributes;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UInventoryComponent* Inventory;

	UPROPERTY(VisibleAnywhere)
	class UQuestManagerComponent* QuestManager;

	UPROPERTY(VisibleAnywhere)
	class UUpgradeManagerComponent* UpgradeManager;

	UPROPERTY()
	TObjectPtr<AActor> PendingInteractable;

	// Extraction zone the player is currently standing in (drives the dedicated Extract key).
	UPROPERTY()
	TObjectPtr<AActor> CurrentExtractionZone;

	UFUNCTION(Server, Reliable) // Reliable - garuntee to be executed. Unreliable - potential to be dropped.
	void ServerEquipButtonPressed();

	// Player Yaw/Pitch Calculations
	float AO_Yaw;
	FRotator StartingAimRotation;

	UPROPERTY()
	class ADefaultPlayerController* PlayerController;

	// Resolves the player's HUD (null if unavailable).
	class APlayerHUD* GetPlayerHUD() const;

	// Builds the "Press [X] to Extract" prompt, resolving the actual key bound to ExtractAction.
	FText BuildExtractPromptText() const;
public:
	// FORCEINLINE:
	// Inlining a function makes the preprocessor replace each function call to it with the body of the function itself.
	// It speeds up the program at runtime at the expense of increasing the size of a translation unit (compiled file).
	// FORCEINLINE will force the compiler to inline a function, taking the choice away from the compiler and leaving it up to the programmer.
	// aka a setter function
	// FORCEINLINE void SetOverlappingWeapon(AWeapon* Weapon){ OverlappingWeapon = Weapon; }

	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	void SetPendingInteractable(AActor* Interactable);
	void ClearPendingInteractableIfMatch(AActor* Interactable);

	void SetExtractionZone(AActor* Zone);
	void ClearExtractionZoneIfMatch(AActor* Zone);

	// Stop/allow movement during the extraction sell summary. Unfreeze is driven by the summary's
	// Continue button (USellSummaryWidget).
	void FreezeForExtraction();
	void UnfreezeAfterExtraction();

	FORCEINLINE class UInventoryComponent* GetInventory() const { return Inventory; }
	FORCEINLINE class UQuestManagerComponent* GetQuestManager() const { return QuestManager; }

	// For calling ONLY ON THE SERVER
	void Eliminated();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();
	
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE bool IsElimmed() const { return bEliminated; }

	// Health now lives in UHeroAttributeSet. These keep their old signatures deliberately, so
	// every existing caller (ADefaultPlayerController's HUD push, AWeapon's damage scaling)
	// compiles and behaves exactly as before.
	float GetHealth() const;
	float GetMaxHealth() const;

	/** The ability system component driving this character's attributes and abilities. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Adds XP towards the current level, rolling over and levelling up as needed.
	// Public so quests / pickups / kill rewards can grant XP without depending on each other.
	void AddXP(float Amount);

	/**
	 *  Adds to the ultimate meter. Called on enemy kills only — quest and pickup XP deliberately
	 *  do not charge ultimates, since the design is "build up from getting kills".
	 *  Safe to call on a character with no ultimate; the attribute simply clamps.
	 */
	void GrantUltimateCharge(float Amount);

	// Grants DOS$ (e.g. coin pickups). BlueprintCallable so pickup BPs can award currency.
	UFUNCTION(BlueprintCallable, Category = "PlayerStats")
	void AddDOSCoins(int32 Amount);

	// bDeferred routes high-frequency writes (coin/XP tick) through the coalescing save path;
	// checkpoints (level-up, sell, etc.) leave it false so they flush immediately.
	void SavePlayerData(bool bDeferred = false);
	void LoadPlayerData();
	void SellLoot();

	// TECH_DEBT(TD-BUG-15): cheat commands compiled into every configuration, Shipping included.
	// They should sit behind #if !UE_BUILD_SHIPPING or move to a UCheatManager.
	UFUNCTION(Exec) void WipeSave();
	UFUNCTION(Exec) void SetLevel(int32 NewLevel);
	UFUNCTION(Exec) void SetDOSCoins(int32 Amount);

	// Called by AExtractionZone on overlap — implement in BP_PlayerCharacter to show/hide prompt
	// TECH_DEBT(TD-BP-1): no C++ fallback. If BP_PlayerCharacter doesn't implement these they are
	// silent no-ops, and nothing tells a tester the overlap fired.
	UFUNCTION(BlueprintImplementableEvent)
	void OnEnteredExtractionZone();
	UFUNCTION(BlueprintImplementableEvent)
	void OnExitedExtractionZone();

	FORCEINLINE int32 GetLevel() const { return Level; }
	FORCEINLINE float GetXP() const { return XP; }
	FORCEINLINE float GetXPToNextLevel() const { return BaseXP + (Level - 1) * XPPerLevel; }
	FORCEINLINE int32 GetDOSCoins() const { return DOSCoins; }
	float GetDamageMultiplier() const;
};
