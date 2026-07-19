// No Rights Reserved @ Team Expedition

#include "AI/EnemyCharacter.h"
#include "ExpProDev.h"
#include "AI/EnemyAIController.h"
#include "Character/PlayerCharacter.h"
#include "PlayerController/DefaultPlayerController.h"
#include "Inventory/ItemPickup.h"
#include "Quest/QuestManagerComponent.h"
#include "HUD/EnemyHealthBarWidget.h"
#include "Components/WidgetComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AEnemyAIController::StaticClass();

	// Floating, camera-facing health bar above the head. Screen space keeps it always readable
	// and a constant size regardless of distance. Assign its Widget Class in the enemy BP.
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetDrawSize(FVector2D(100.f, 10.f));
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	HealthBarWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Hidden until the enemy takes its first hit (revealed in UpdateHealthBar).
	HealthBarWidget->SetHiddenInGame(true);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,     ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (AnimBP.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBP.Class);
	}
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	OnTakeAnyDamage.AddDynamic(this, &AEnemyCharacter::ReceiveDamage);
	UpdateHealthBar();
}

void AEnemyCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHealthBar();

	if (Health > 0.f)
	{
		if (InstigatedBy)
			if (AEnemyAIController* EnemyAI = Cast<AEnemyAIController>(GetController()))
				EnemyAI->ForceChase(InstigatedBy->GetPawn());
		return;
	}

	// Grant XP and increment kill counter on the player who landed the killing blow
	if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(InstigatedBy))
	{
		PC->AddKill();
		if (APlayerCharacter* Killer = Cast<APlayerCharacter>(PC->GetPawn()))
		{
			Killer->AddXP(XPReward);
			if (UQuestManagerComponent* QM = Killer->GetQuestManager())
				QM->NotifyEnemyKilled(GetClass());
		}
	}

	DropLoot();
	Destroy();
}

void AEnemyCharacter::UpdateHealthBar()
{
	if (!HealthBarWidget) return;

	const float Percent = MaxHealth > 0.f ? Health / MaxHealth : 0.f;

	// Only show the bar once the enemy has actually taken damage.
	HealthBarWidget->SetHiddenInGame(Percent >= 1.f);

	if (UEnemyHealthBarWidget* Bar = Cast<UEnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject()))
	{
		Bar->SetHealthPercent(Percent);
	}
}

void AEnemyCharacter::DropLoot()
{
	FVector DropLocation = GetActorLocation();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FLootEntry& Entry : LootTable.Entries)
	{
		if (!Entry.Item || !Entry.PickupActorClass) continue;
		if (FMath::FRand() > Entry.DropChance) continue;

		const int32 Qty = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
		if (AItemPickup* Pickup = GetWorld()->SpawnActor<AItemPickup>(
			Entry.PickupActorClass, DropLocation, FRotator::ZeroRotator, Params))
		{
			Pickup->ItemDef = Entry.Item;
			Pickup->Quantity = Qty;
			DropLocation.X += 50.f;
		}
	}
}
