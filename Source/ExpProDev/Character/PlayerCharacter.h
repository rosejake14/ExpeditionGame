// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"

#include "PlayerCharacter.generated.h"



UCLASS()
class EXPPRODEV_API APlayerCharacter : public ACharacter
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

	//	Blueprint Editable Variables

	UPROPERTY(EditAnywhere, Category = "Audio")
	class USoundBase* LevelUpSound;

	// Player Health:
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float MaxHealth = 100.f;

	// Captured from MaxHealth in PostInitializeComponents — upgrades are added on top of this
	float BaseMaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadWrite, EditAnywhere, Category = PlayerStats)
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health();

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

	// Multiplier applied to all outgoing damage — set by UpgradeManagerComponent on load
	UPROPERTY(BlueprintReadOnly, Category = PlayerStats)
	float DamageMultiplier = 1.0f;

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
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE bool IsElimmed() const { return bEliminated; }

	// Adds XP towards the current level, rolling over and levelling up as needed.
	// Public so quests / pickups / kill rewards can grant XP without depending on each other.
	void AddXP(float Amount);

	// Grants DOS$ (e.g. coin pickups). BlueprintCallable so pickup BPs can award currency.
	UFUNCTION(BlueprintCallable, Category = "PlayerStats")
	void AddDOSCoins(int32 Amount);

	// bDeferred routes high-frequency writes (coin/XP tick) through the coalescing save path;
	// checkpoints (level-up, sell, etc.) leave it false so they flush immediately.
	void SavePlayerData(bool bDeferred = false);
	void LoadPlayerData();
	void SellLoot();

	UFUNCTION(Exec) void WipeSave();
	UFUNCTION(Exec) void SetLevel(int32 NewLevel);
	UFUNCTION(Exec) void SetDOSCoins(int32 Amount);

	// Called by AExtractionZone on overlap — implement in BP_PlayerCharacter to show/hide prompt
	UFUNCTION(BlueprintImplementableEvent)
	void OnEnteredExtractionZone();
	UFUNCTION(BlueprintImplementableEvent)
	void OnExitedExtractionZone();

	FORCEINLINE int32 GetLevel() const { return Level; }
	FORCEINLINE float GetXP() const { return XP; }
	FORCEINLINE float GetXPToNextLevel() const { return BaseXP + (Level - 1) * XPPerLevel; }
	FORCEINLINE int32 GetDOSCoins() const { return DOSCoins; }
	FORCEINLINE float GetDamageMultiplier() const { return DamageMultiplier; }
};
