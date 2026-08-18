// No Rights Reserved @ Team Expedition 

#include "Weapon/Weapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false; // Doesn't need to tick.

	// We want it to only replicate on the server. This also means that we can have replicating variables.
	bReplicates = true;
	
	// Mesh
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	// Using CCD to avoid issues with Weapon Meshes falling through the ground on clients.
	// TECH_DEBT(TD-BUG-22): CCD is forced on for every weapon at all times to paper over dropped
	// weapons tunnelling through the floor. It should be scoped to the dropped/simulating state,
	// or the collision setup fixed so it isn't needed at all.
	WeaponMesh->SetUseCCD(true);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Collision
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	// We want to do these collisions on the SERVER - so on all machines, they will have no collision. 
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore); 
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// HUD
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	if (HasAuthority()) // If server role, then the replicating actor will have collision. 
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter) // && PickupWidget)
	{
		// PickupWidget->SetVisibility(true);
		PlayerCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnRep_WeaponState() // This will trigger when the Weapon State changes on the clients. RPC
{
	switch (WeaponState)
	{
		case(EWeaponState::EWS_Equipped):
			ShowPickupWidget(false);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
			break;
		
		case(EWeaponState::EWS_Dropped):
			WeaponMesh->SetSimulatePhysics(true);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
			break;
		
	default:
		break;
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

float AWeapon::GetOutgoingDamage() const
{
	const APlayerCharacter* OwnerPC = Cast<APlayerCharacter>(GetOwner());
	const float Multiplier = OwnerPC ? OwnerPC->GetDamageMultiplier() : 1.0f;
	return BaseDamage * Multiplier;
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireSound)
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());

	ApplyImpact(HitTarget);
}

void AWeapon::ApplyImpact(const FVector& HitTarget)
{
	if (BaseDamage <= 0.f || !GetWorld()) return;

	// TECH_DEBT(TD-BUG-4): UCombatComponent::TraceUnderCrosshairs already resolved the actor under
	// the crosshair, but only the impact POINT is passed through — so this second trace, from the
	// weapon actor's origin rather than the camera, can hit a different actor (or be blocked by
	// cover the camera trace saw past). Pass the resolved FHitResult through instead.

	// Trace from weapon location to the confirmed hit point to find the target actor
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), HitTarget, ECC_Visibility, Params);

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor) return;

	const APlayerCharacter* OwnerPC = Cast<APlayerCharacter>(GetOwner());
	AController* OwnerController = OwnerPC ? OwnerPC->GetController() : nullptr;

	UGameplayStatics::ApplyDamage(HitActor, GetOutgoingDamage(), OwnerController, this, UDamageType::StaticClass());
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	// Detach from owner component
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	
	// Need to remove the last character's owner when it's dropped
	SetOwner(nullptr); 
}

void AWeapon::SetWeaponState(EWeaponState State) // This is setting it on the server
{
	WeaponState = State;
	
	switch (WeaponState)
	{
	case(EWeaponState::EWS_Equipped):
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);

		break;

	case(EWeaponState::EWS_Dropped):
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		break;

	default:
		break;
	}
}

