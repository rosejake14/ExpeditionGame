// No Rights Reserved @ Team Expedition

#include "Extraction/ExtractionZone.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Character/PlayerCharacter.h"
#include "Weapon/WeaponRegistry.h"
#include "Weapon/WeaponDefinition.h"
#include "Weapon/Weapon.h"
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
			// Random point within the box footprint, traced down to the ground
			const FVector CandidateXY(
				Origin.X + FMath::FRandRange(-Extent.X, Extent.X),
				Origin.Y + FMath::FRandRange(-Extent.Y, Extent.Y),
				Origin.Z);

			FVector SpawnLocation = CandidateXY;

			FHitResult Hit;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(Hit,
				CandidateXY + FVector(0.f, 0.f, GroundTraceHeight),
				CandidateXY - FVector(0.f, 0.f, GroundTraceHeight),
				ECC_WorldStatic, QueryParams))
			{
				SpawnLocation = Hit.ImpactPoint;
			}

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
