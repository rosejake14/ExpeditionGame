// No Rights Reserved @ Team Expedition 


#include "Weapon/ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    FVector NormalImpulse, const FHitResult& HitResult)
{
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
