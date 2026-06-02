// No Rights Reserved @ Team Expedition 

#include "Weapon/WeaponProjectile.h"
#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AWeaponProjectile::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("Muzzle"));

	if (MuzzleSocket)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
		
		// From the muzzle socket to the hit location from TraceUnderCrosshairs()
		FVector TargetLocation = HitTarget - SocketTransform.GetLocation();

		// Spawn 50 units forward to avoid self-collision when moving/running
		FVector ProjectileDirection = (HitTarget - SocketTransform.GetLocation()).GetSafeNormal();
		FVector SpawnLocation = SocketTransform.GetLocation() + (ProjectileDirection * SpawnLocationOffset);
		
		FRotator TargetRotation = TargetLocation.Rotation();
		
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, TargetRotation, SpawnParameters);
			}
		}
	}
}
