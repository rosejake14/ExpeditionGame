// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EXPPRODEV_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	friend class APlayerCharacter;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	
protected:
	virtual void BeginPlay() override;

	void FireButtonPressed(bool bPressed);
	void SetAiming(bool bIsAiming);

	// Server RPC's
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize HitTarget);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize HitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshair(float DeltaTime);

	UFUNCTION()
	void OnRep_EquippedWeapon();

private:
	class APlayerCharacter* PlayerCharacter;
	class ADefaultPlayerController* PlayerController;
	class APlayerHUD* PlayerHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bFireButtonPressed;
	
	UPROPERTY(Replicated)
	bool bAiming;

	


};
