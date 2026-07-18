// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "WeaponProjectile.generated.h"

/**
 * 
 */
UCLASS()
class EXPPRODEV_API AWeaponProjectile : public AWeapon
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere)
	float SpawnLocationOffset = 10.f;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	// No-op: projectile weapons deal damage through the spawned projectile, not a hitscan trace.
	virtual void ApplyImpact(const FVector& HitTarget) override {}
};
