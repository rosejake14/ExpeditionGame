// No Rights Reserved @ Team Expedition

#include "Inventory/ItemPickup.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/InventoryComponent.h"
#include "Character/PlayerCharacter.h"
#include "Components/SphereComponent.h"

AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;

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
	BaseZ = GetActorLocation().Z;
	PhaseOffset = FMath::RandRange(0.f, 2.f * PI);
	SetActorRotation(FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f));
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnSphereOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AItemPickup::OnSphereEndOverlap);
}

void AItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Loc = GetActorLocation();
	Loc.Z = BaseZ + FMath::Sin(GetWorld()->GetTimeSeconds() * FloatSpeed + PhaseOffset) * FloatAmplitude;
	SetActorLocation(Loc);

	AddActorWorldRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
}

void AItemPickup::Interact(APlayerCharacter* Player)
{
	if (!Player || !ItemDef) return;
	UInventoryComponent* Inv = Player->GetInventory();
	if (Inv && Inv->AddItem(ItemDef, Quantity))
	{
		Player->ClearPendingInteractableIfMatch(this);
		Destroy();
	}
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
			Destroy();
	}
	else
	{
		Player->SetPendingInteractable(this);
	}
}

void AItemPickup::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
		Player->ClearPendingInteractableIfMatch(this);
}
