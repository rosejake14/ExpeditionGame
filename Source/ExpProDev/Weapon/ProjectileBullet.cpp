// No Rights Reserved @ Team Expedition 


#include "Weapon/ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    FVector NormalImpulse, const FHitResult& HitResult)
{
    // TECH_DEBT(TD-BUG-5): damage is applied to whatever the bullet hits, with no filtering — the
    // shooter, other players and world geometry all take a damage event. Only the hitmarker sound
    // below bothers to exclude the owner. Needs an owner/team filter on the damage call itself.
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        AController* OwnerController = OwnerCharacter->GetController();
        if (OwnerController)
        {
            UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
        }
    }

    if (HitmarkerSound && OtherActor && OtherActor != GetOwner() && Cast<ACharacter>(OtherActor))
        UGameplayStatics::PlaySound2D(this, HitmarkerSound);

    // Because we destroy the bullet, this Super:: should be called last, or none of the code below it will execute.
    Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, HitResult);
}
