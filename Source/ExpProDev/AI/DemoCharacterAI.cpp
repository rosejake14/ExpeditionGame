// No Rights Reserved @ Team Expedition 


#include "AI/DemoCharacterAI.h"

#include "ExpProDev.h"
#include "AI/AICharacterController.h"
#include "Gamemode/DefaultGameMode.h"
#include "Perception/AIPerceptionComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/DefaultPlayerController.h"
#include "Inventory/ItemPickup.h"
#include "Character/PlayerCharacter.h"

// Sets default values
ADemoCharacterAI::ADemoCharacterAI()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}

void ADemoCharacterAI::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADemoCharacterAI, Health);
}

// Called when the game starts or when spawned
void ADemoCharacterAI::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	// If on server, and a proj bullet hits, it will recieve damage.
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ADemoCharacterAI::ReceiveDamage);
	}
}

void ADemoCharacterAI::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	class AController* InstigatedController, class AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage,0.f, MaxHealth);

	//
	// TODO: RoseJ - Implement Hit React Montage
	// PlayHitReactMontage();
	//

	// TODO: Implement Enemy health Bar overlay
	//UpdateHUDHealth();
	// Link to Gamemode, eliminate from there.
	
	if (Health <= 0.f)
	{
		ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(InstigatedController);
		if (PC)
		{
			PC->AddKill();

			APlayerCharacter* KillerCharacter = Cast<APlayerCharacter>(PC->GetPawn());
			if (KillerCharacter)
			{
				KillerCharacter->AddXP(XPReward);
			}
		}

		// Spawn loot at the AI's death location
		FVector SpawnLocation = GetActorLocation();
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		for (const FLootEntry& Entry : LootTable.Entries)
		{
			if (!Entry.Item || !Entry.PickupActorClass) continue;
			if (FMath::FRand() <= Entry.DropChance)
			{
				int32 Qty = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
				AItemPickup* Pickup = GetWorld()->SpawnActor<AItemPickup>(Entry.PickupActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (Pickup)
				{
					Pickup->ItemDef = Entry.Item;
					Pickup->Quantity = Qty;
					SpawnLocation += FVector(50.f, 0.f, 0.f);
				}
			}
		}

		Destroy();
	}
	
}

// Called every frame
void ADemoCharacterAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADemoCharacterAI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

