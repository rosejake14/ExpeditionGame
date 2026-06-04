// No Rights Reserved @ Team Expedition

#include "Inventory/ItemPickup.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/InventoryComponent.h"
#include "Character/PlayerCharacter.h"
#include "Components/SphereComponent.h"

AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetSphereRadius(100.f);
	RootComponent = SphereCollision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemPickup::BeginPlay()
{
	Super::BeginPlay();
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnSphereOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AItemPickup::OnSphereEndOverlap);
}

void AItemPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player || !ItemDef) return;

	if (ItemDef->bAutoPickup)
	{
		UInventoryComponent* Inv = Player->GetInventory();
		if (Inv && Inv->AddItem(ItemDef, Quantity))
		{
			Destroy();
		}
	}
	else
	{
		Player->SetPendingPickup(this);
	}
}

void AItemPickup::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		Player->ClearPendingPickupIfMatch(this);
	}
}
