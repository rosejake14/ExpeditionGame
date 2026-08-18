// No Rights Reserved @ Team Expedition

#include "Extraction/ExtractionZone.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Character/PlayerCharacter.h"
#include "Weapon/WeaponRegistry.h"
#include "Weapon/WeaponDefinition.h"
#include "Weapon/Weapon.h"
#include "Inventory/ItemPickup.h"
#include "Save/ExpProSaveGame.h"
#include "Save/SaveGameSubsystem.h"

AExtractionZone::AExtractionZone()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	SetRootComponent(BoxCollision);

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(RootComponent);
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinder<UTexture2D> Texture;
			FConstructorStatics() : Texture(TEXT("/Engine/EditorResources/S_Trigger")) {}
		};
		static FConstructorStatics ConstructorStatics;
		if (ConstructorStatics.Texture.Succeeded())
		{
			SpriteComponent->SetSprite(ConstructorStatics.Texture.Object);
		}
	}
#endif
}

void AExtractionZone::BeginPlay()
{
	Super::BeginPlay();
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AExtractionZone::OnBoxOverlap);
	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &AExtractionZone::OnBoxEndOverlap);

	SpawnPurchasedWeapons();
}

// TECH_DEBT(TD-BUG-11): this runs per-zone at BeginPlay and reads the shared purchased-weapon
// counts without claiming them. Two extraction zones on one map therefore each spawn the FULL
// purchased set, duplicating everything the player bought. The save is only decremented on pickup
// (UCombatComponent::EquipWeapon), so the duplicates are free.
void AExtractionZone::SpawnPurchasedWeapons()
{
	if (!WeaponRegistry) return;

	UExpProSaveGame* Save = USaveGameSubsystem::LoadActiveSlot(this);
	if (!Save || Save->PurchasedWeapons.Num() == 0) return;

	const FVector Origin = BoxCollision->GetComponentLocation();
	const FVector Extent = BoxCollision->GetScaledBoxExtent();

	for (const TPair<FName, int32>& Pair : Save->PurchasedWeapons)
	{
		UWeaponDefinition* Def = WeaponRegistry->FindById(Pair.Key);
		if (!Def || !Def->WeaponClass) continue;

		for (int32 i = 0; i < Pair.Value; ++i)
		{
			// Random point within the box footprint, dropped onto the floor beneath it.
			const FVector CandidateXY(
				Origin.X + FMath::FRandRange(-Extent.X, Extent.X),
				Origin.Y + FMath::FRandRange(-Extent.Y, Extent.Y),
				Origin.Z);

			const FVector SpawnLocation = AItemPickup::GroundedLocation(GetWorld(), CandidateXY, this);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			if (AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(
				Def->WeaponClass, SpawnLocation, FRotator::ZeroRotator, Params))
			{
				// Tag it so picking it up consumes one unit from the save
				Weapon->ConsumeWeaponId = Def->WeaponId;
			}
		}
	}
}

void AExtractionZone::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->SetExtractionZone(this);
		Player->OnEnteredExtractionZone();
	}
}

void AExtractionZone::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->ClearExtractionZoneIfMatch(this);
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
