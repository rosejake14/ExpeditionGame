// No Rights Reserved @ Team Expedition

#include "Quest/ObjectReturnPoint.h"
#include "Quest/QuestManagerComponent.h"
#include "Quest/QuestDefinition.h"
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
	if (!bEnabled) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	UQuestManagerComponent* QM = Player->GetQuestManager();
	if (!QM || !QM->HasActiveQuest()) return;

	UQuestDefinition* Target = LinkedQuest ? LinkedQuest.Get() : QM->GetActiveQuestDefinition();
	if (QM->IsItemCollectedFor(Target))
		QM->NotifyQuestComplete(Target);
}
