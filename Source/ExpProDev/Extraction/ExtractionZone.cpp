// No Rights Reserved @ Team Expedition

#include "Extraction/ExtractionZone.h"
#include "Components/BoxComponent.h"
#include "Character/PlayerCharacter.h"

AExtractionZone::AExtractionZone()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	SetRootComponent(BoxCollision);
}

void AExtractionZone::BeginPlay()
{
	Super::BeginPlay();
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AExtractionZone::OnBoxOverlap);
	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &AExtractionZone::OnBoxEndOverlap);
}

void AExtractionZone::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->SetPendingInteractable(this);
		Player->OnEnteredExtractionZone();
	}
}

void AExtractionZone::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->ClearPendingInteractableIfMatch(this);
		Player->OnExitedExtractionZone();
	}
}

void AExtractionZone::Interact(APlayerCharacter* Player)
{
	if (Player)
	{
		Player->SellLoot();
	}
}
