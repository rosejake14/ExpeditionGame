// No Rights Reserved @ Team Expedition

#include "Quest/QuestGiverNPC.h"
#include "Quest/QuestDefinition.h"
#include "Character/PlayerCharacter.h"
#include "HUD/PlayerHUD.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"

AQuestGiverNPC::AQuestGiverNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractRadius = CreateDefaultSubobject<USphereComponent>(TEXT("InteractRadius"));
	InteractRadius->SetupAttachment(RootComponent);
	InteractRadius->SetSphereRadius(200.f);
	InteractRadius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AQuestGiverNPC::BeginPlay()
{
	Super::BeginPlay();
	InteractRadius->OnComponentBeginOverlap.AddDynamic(this, &AQuestGiverNPC::OnSphereBeginOverlap);
	InteractRadius->OnComponentEndOverlap.AddDynamic(this, &AQuestGiverNPC::OnSphereEndOverlap);
}

void AQuestGiverNPC::Interact(APlayerCharacter* Player)
{
	if (!Player || AvailableQuests.IsEmpty()) return;

	APlayerController* PC = Player->GetController<APlayerController>();
	if (!PC) return;

	APlayerHUD* HUD = Cast<APlayerHUD>(PC->GetHUD());
	if (!HUD) return;

	TArray<UQuestDefinition*> Quests;
	for (TObjectPtr<UQuestDefinition>& Q : AvailableQuests)
		if (Q) Quests.Add(Q);

	HUD->ShowQuestSelection(Player, Quests);
}

void AQuestGiverNPC::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
		Player->SetPendingInteractable(this);
}

void AQuestGiverNPC::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
		Player->ClearPendingInteractableIfMatch(this);
}
