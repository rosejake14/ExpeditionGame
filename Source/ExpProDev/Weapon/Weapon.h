// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_InitialState UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_Max UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class EXPPRODEV_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShowWidget);

	// Plays shared fire cosmetics, then applies this weapon's impact (hitscan by default).
	// Projectile weapons override Fire to spawn a projectile and skip the hitscan impact.
	virtual void Fire(const FVector& HitTarget);

	void Dropped();

	// Final outgoing damage = BaseDamage scaled by the owning player's DamageMultiplier (upgrades).
	// Shared so hitscan and projectile paths use one source of truth.
	float GetOutgoingDamage() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "!Weapon Properties")
	float BaseDamage = 20.f;

	// When set, this weapon was spawned from a purchased (consumable) unit.
	// On equip it decrements that weapon's saved quantity. NAME_None for level-placed weapons.
	UPROPERTY(BlueprintReadWrite, Category = "!Weapon Properties")
	FName ConsumeWeaponId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Audio")
	class USoundBase* FireSound;

	// Textures for the weapon crosshairs:
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairCentre;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairTop;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairBottom;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairRight;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairLeft;
protected:
	virtual void BeginPlay() override;

	// Traces from the muzzle to HitTarget and applies GetOutgoingDamage() to what it hits.
	// Overridden to a no-op by projectile weapons so damage isn't dealt twice.
	virtual void ApplyImpact(const FVector& HitTarget);

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);

private:
	UPROPERTY(VisibleAnywhere, Category = "!Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "!Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "!Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "!Weapon Properties")
	class UWidgetComponent* PickupWidget;

public:
	// FORCEINLINE void SetWeaponState(EWeaponState State) {WeaponState = State;}
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
