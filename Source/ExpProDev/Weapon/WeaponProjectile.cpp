// No Rights Reserved @ Team Expedition 

#include "Weapon/WeaponProjectile.h"
#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AWeaponProjectile::Fire(const FVector& HitTarget)
{
	// Super plays fire cosmetics; ApplyImpact is overridden to a no-op so no hitscan damage is dealt.
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("Muzzle"));

	if (MuzzleSocket)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());

		// From the muzzle socket to the hit location from TraceUnderCrosshairs()
		FVector TargetLocation = HitTarget - SocketTransform.GetLocation();

		// Spawn slightly forward to avoid self-collision when moving/running
		FVector ProjectileDirection = (HitTarget - SocketTransform.GetLocation()).GetSafeNormal();
		FVector SpawnLocation = SocketTransform.GetLocation() + (ProjectileDirection * SpawnLocationOffset);

		FRotator TargetRotation = TargetLocation.Rotation();

		if (ProjectileClass && InstigatorPawn)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				// Deferred spawn so the projectile's damage is set (from the weapon's upgrade-scaled
				// value) before BeginPlay/collision can run.
				const FTransform SpawnTransform(TargetRotation, SpawnLocation);
				AProjectile* Projectile = World->SpawnActorDeferred<AProjectile>(
					ProjectileClass, SpawnTransform, GetOwner(), InstigatorPawn,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if (Projectile)
				{
					Projectile->SetDamage(GetOutgoingDamage());
					UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);
				}
			}
		}
	}
}
