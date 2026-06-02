// No Rights Reserved @ Team Expedition 
//
// ALlows for multiple ways of applying damage depending on the class/type of project
//

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class EXPPRODEV_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;

};
