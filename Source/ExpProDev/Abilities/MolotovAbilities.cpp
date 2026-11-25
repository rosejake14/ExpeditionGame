// No Rights Reserved @ Team Expedition

#include "Abilities/MolotovAbilities.h"

#include "ExpProDev.h"
#include "Hero/HeroCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** How far the crosshair trace reaches when picking the throw's aim point. */
	constexpr float AimTraceDistance = 100000.f;

	/** How far below an impact to look for the ground the fire sits on. */
	constexpr float GroundTraceDistance = 5000.f;

	/** Pawns overlapping a sphere, de-duplicated (one pawn can report several components). */
	TSet<APawn*> FindPawnsInSphere(const UWorld* World, const FVector& Centre, float Radius)
	{
		TSet<APawn*> Pawns;
		if (!World) return Pawns;

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(Overlaps, Centre, FQuat::Identity,
			FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(Radius));

		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (APawn* Pawn = Cast<APawn>(Overlap.GetActor()))
			{
				Pawns.Add(Pawn);
			}
		}
		return Pawns;
	}
}

// ── Throw Molotov (Power) ─────────────────────────────────────────────────

UHeroAbility_ThrowMolotov::UHeroAbility_ThrowMolotov()
{
	CooldownSeconds = 5.f;
	DisplayName = NSLOCTEXT("HeroAbilities", "ThrowMolotov", "Molotov");
	MolotovClass = AMolotovProjectile::StaticClass();
	FireClass = AMolotovFire::StaticClass();
}

void UHeroAbility_ThrowMolotov::ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo)
{
	AHeroCharacter* Hero = GetHeroCharacter(ActorInfo);
	UWorld* World = Hero ? Hero->GetWorld() : nullptr;
	if (!World || !MolotovClass) return;

	// Spawn just in front of the middle of the mesh.
	const FVector MeshCentre = Hero->GetMesh() ? Hero->GetMesh()->Bounds.Origin : Hero->GetActorLocation();
	const FVector SpawnLocation = MeshCentre + Hero->GetActorForwardVector() * SpawnForwardOffset;

	// Aim at whatever is under the crosshair: trace from the camera along the view direction.
	FVector ViewLocation = Hero->GetActorLocation();
	FRotator ViewRotation = Hero->GetControlRotation();
	if (AController* Controller = Hero->GetController())
	{
		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * AimTraceDistance;
	FHitResult AimHit;
	FCollisionQueryParams AimParams(SCENE_QUERY_STAT(MolotovAim), false, Hero);
	const FVector AimPoint = World->LineTraceSingleByChannel(AimHit, ViewLocation, TraceEnd, ECC_Visibility, AimParams)
		? AimHit.ImpactPoint : TraceEnd;

	const FVector Direction = (AimPoint - SpawnLocation).GetSafeNormal();
	const FTransform SpawnTransform(Direction.Rotation(), SpawnLocation);

	AMolotovProjectile* Molotov = World->SpawnActorDeferred<AMolotovProjectile>(
		MolotovClass, SpawnTransform, Hero, Hero, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Molotov) return;

	FMolotovFireSettings Fire;
	Fire.Radius       = FireRadius;
	Fire.Duration     = FireDuration;
	Fire.TickDamage   = GetScaledDamage(FireTickDamage);
	Fire.TickInterval = FireTickInterval;
	Fire.Height       = FireHeight;

	Molotov->InitMolotov(ThrowSpeed, GetScaledDamage(ImpactDamage), FireClass, Fire);
	Molotov->FinishSpawning(SpawnTransform);

	UE_LOG(LogExpProDev, Log, TEXT("ThrowMolotov: thrown at %.0f cm/s, direction %s."),
		ThrowSpeed, *Direction.ToCompactString());
}

// ── Molotov projectile ────────────────────────────────────────────────────

AMolotovProjectile::AMolotovProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 10.f; // a molotov thrown off the map cleans itself up

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(10.f);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	SetRootComponent(Collision);

	// Placeholder visual: the engine's basic sphere (100 cm across), scaled to the 20 cm collision.
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.2f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->ProjectileGravityScale = 1.f;
	Movement->bShouldBounce = false;
	Movement->bRotationFollowsVelocity = true;
}

void AMolotovProjectile::InitMolotov(float InSpeed, float InImpactDamage, TSubclassOf<AMolotovFire> InFireClass,
	const FMolotovFireSettings& InFireSettings)
{
	Movement->InitialSpeed = InSpeed;
	Movement->MaxSpeed = InSpeed;
	ImpactDamage = InImpactDamage;
	FireClass = InFireClass;
	FireSettings = InFireSettings;
}

void AMolotovProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Never collide with the thrower.
	if (AActor* Thrower = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(Thrower, true);
	}

	Collision->OnComponentHit.AddDynamic(this, &AMolotovProjectile::OnHit);
}

void AMolotovProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (bBroken) return;
	bBroken = true;

	UE_LOG(LogExpProDev, Log, TEXT("Molotov: hit %s."), *GetNameSafe(OtherActor));

	// Impact damage is direct-hit only.
	APawn* HitPawn = Cast<APawn>(OtherActor);
	if (HitPawn && HitPawn != GetInstigator() && ImpactDamage > 0.f)
	{
		UGameplayStatics::ApplyDamage(HitPawn, ImpactDamage, GetInstigatorController(), this, UDamageType::StaticClass());
	}

	// The fire always sits on the ground below the impact — walls and enemies included. Tracing
	// static/dynamic world geometry only means pawns can't be mistaken for the floor.
	UWorld* World = GetWorld();
	const FVector TraceStart = Hit.ImpactPoint + FVector(0.f, 0.f, 5.f);
	FHitResult Ground;
	FCollisionObjectQueryParams GroundObjects;
	GroundObjects.AddObjectTypesToQuery(ECC_WorldStatic);
	GroundObjects.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(MolotovGround), false, this);

	const bool bFoundGround = World && World->LineTraceSingleByObjectType(
		Ground, TraceStart, TraceStart - FVector(0.f, 0.f, GroundTraceDistance), GroundObjects, GroundParams);

	if (bFoundGround && FireClass)
	{
		const FTransform FireTransform(FRotator::ZeroRotator, Ground.ImpactPoint);
		AMolotovFire* Fire = World->SpawnActorDeferred<AMolotovFire>(
			FireClass, FireTransform, GetOwner(), GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Fire)
		{
			Fire->InitFire(FireSettings);
			Fire->FinishSpawning(FireTransform);
		}
	}
	else
	{
		UE_LOG(LogExpProDev, Warning, TEXT("Molotov: no ground within %.0f cm below the impact, no fire."), GroundTraceDistance);
	}

	Destroy();
}

// ── Molotov fire ──────────────────────────────────────────────────────────

AMolotovFire::AMolotovFire()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AMolotovFire::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(Settings.Duration);

	UE_LOG(LogExpProDev, Log, TEXT("MolotovFire: started at %s (%.0f radius, %.1fs)."),
		*GetActorLocation().ToCompactString(), Settings.Radius, Settings.Duration);

	OnFireStarted(Settings.Radius, Settings.Duration);

	// Tick straight away, then every interval.
	TickFire();
	GetWorldTimerManager().SetTimer(TickTimer, this, &AMolotovFire::TickFire, Settings.TickInterval, true);
}

void AMolotovFire::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(TickTimer);
	UE_LOG(LogExpProDev, Log, TEXT("MolotovFire: burned out."));
	Super::EndPlay(EndPlayReason);
}

void AMolotovFire::TickFire()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Centre = GetActorLocation();

	// Flat orange ring on the ground, lasting one tick so it reads as continuous.
	DrawDebugCircle(World, Centre + FVector(0.f, 0.f, 5.f), Settings.Radius, 48, FColor::Orange,
		false, Settings.TickInterval, 0, 3.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);

	int32 HitCount = 0;
	for (APawn* Pawn : FindPawnsInSphere(World, Centre, Settings.Radius))
	{
		if (Pawn == GetInstigator()) continue; // no self-damage
		if (Pawn->GetActorLocation().Z - Centre.Z > Settings.Height) continue;

		UGameplayStatics::ApplyDamage(Pawn, Settings.TickDamage, GetInstigatorController(), this, UDamageType::StaticClass());
		++HitCount;
	}

	UE_LOG(LogExpProDev, Verbose, TEXT("MolotovFire: tick hit %d pawns."), HitCount);
}

// ── Gunpowder Burp (Ultimate) ─────────────────────────────────────────────

UHeroAbility_GunpowderBurp::UHeroAbility_GunpowderBurp()
{
	bChargeGated = true;
	bEndsImmediately = false;
	DisplayName = NSLOCTEXT("HeroAbilities", "GunpowderBurp", "Gunpowder Burp");
}

void UHeroAbility_GunpowderBurp::ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo)
{
	AHeroCharacter* Hero = GetHeroCharacter(ActorInfo);
	UWorld* World = Hero ? Hero->GetWorld() : nullptr;
	if (!World)
	{
		FinishAbility(true);
		return;
	}

	BurpHero = Hero;
	if (const UCharacterMovementComponent* Movement = Hero->GetCharacterMovement())
	{
		SavedMaxWalkSpeed = Movement->MaxWalkSpeed;
	}

	UE_LOG(LogExpProDev, Log, TEXT("GunpowderBurp: started (%.1fs, %.0f dmg every %.2fs)."),
		BurpDuration, BurpTickDamage, BurpTickInterval);

	// First tick straight away, then every interval; a separate timer ends the burp.
	BurpTick();
	World->GetTimerManager().SetTimer(TickTimer, this, &UHeroAbility_GunpowderBurp::BurpTick, BurpTickInterval, true);
	World->GetTimerManager().SetTimer(EndTimer, this, &UHeroAbility_GunpowderBurp::OnBurpFinished, BurpDuration, false);
}

void UHeroAbility_GunpowderBurp::BurpTick()
{
	UWorld* World = BurpHero ? BurpHero->GetWorld() : nullptr;
	if (!World) return;

	// Re-applied every tick so a sprint press mid-burp can't override the slow for long.
	if (UCharacterMovementComponent* Movement = BurpHero->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = SavedMaxWalkSpeed * MoveSpeedMultiplier;
	}

	// From roughly the mouth, along the camera (including pitch).
	const FVector Origin = BurpHero->GetActorLocation() + FVector(0.f, 0.f, BurpHero->BaseEyeHeight);
	const FVector Direction = BurpHero->GetControlRotation().Vector();
	const float HalfAngleRad = FMath::DegreesToRadians(ConeAngle * 0.5f);
	const float MinDot = FMath::Cos(HalfAngleRad);

	DrawDebugCone(World, Origin, Direction, ConeLength, HalfAngleRad, HalfAngleRad, 16, FColor::Orange,
		false, BurpTickInterval, 0, 2.f);

	const float Damage = GetScaledDamage(BurpTickDamage);
	int32 HitCount = 0;

	// No line-of-sight check: the burp passes through walls by design.
	for (APawn* Pawn : FindPawnsInSphere(World, Origin, ConeLength))
	{
		if (Pawn == BurpHero) continue;

		const FVector ToPawn = Pawn->GetActorLocation() - Origin;
		const float Distance = ToPawn.Size();
		if (Distance > ConeLength || Distance <= KINDA_SMALL_NUMBER) continue;
		if (FVector::DotProduct(ToPawn / Distance, Direction) < MinDot) continue;

		UGameplayStatics::ApplyDamage(Pawn, Damage, BurpHero->GetController(), BurpHero, UDamageType::StaticClass());
		++HitCount;
	}

	UE_LOG(LogExpProDev, Verbose, TEXT("GunpowderBurp: tick hit %d pawns."), HitCount);
}

void UHeroAbility_GunpowderBurp::OnBurpFinished()
{
	FinishAbility();
}

void UHeroAbility_GunpowderBurp::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Runs on normal end and on cancel (e.g. death), so speed is always restored.
	if (BurpHero)
	{
		if (UWorld* World = BurpHero->GetWorld())
		{
			World->GetTimerManager().ClearTimer(TickTimer);
			World->GetTimerManager().ClearTimer(EndTimer);
		}

		if (UCharacterMovementComponent* Movement = BurpHero->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = SavedMaxWalkSpeed;
		}

		UE_LOG(LogExpProDev, Log, TEXT("GunpowderBurp: ended%s."), bWasCancelled ? TEXT(" (cancelled)") : TEXT(""));
		BurpHero = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
