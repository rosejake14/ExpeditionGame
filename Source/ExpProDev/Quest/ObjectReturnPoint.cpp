// No Rights Reserved @ Team Expedition

#include "Quest/ObjectReturnPoint.h"
#include "Quest/QuestComponent.h"
#include "Components/SphereComponent.h"
#include "Character/PlayerCharacter.h"

AObjectReturnPoint::AObjectReturnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetSphereRadius(200.f);
	RootComponent = SphereCollision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AObjectReturnPoint::BeginPlay()
{
	Super::BeginPlay();
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AObjectReturnPoint::OnSphereOverlap);
}

void AObjectReturnPoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	UQuestComponent* Quest = Player->FindComponentByClass<UQuestComponent>();
	if (Quest && Quest->IsItemCollected())
	{
		Quest->NotifyQuestComplete();
	}
}
