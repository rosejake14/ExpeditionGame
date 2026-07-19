// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "ItemPickup.generated.h"

UCLASS()
class EXPPRODEV_API AItemPickup : public AActor, public IInteractable
{
	GENERATED_BODY()
public:
	AItemPickup();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	class UItemDefinition* ItemDef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class USoundBase* PickupSound;

	UPROPERTY(EditAnywhere, Category = "Pickup|Animation")
	float FloatAmplitude = 20.f;

	UPROPERTY(EditAnywhere, Category = "Pickup|Animation")
	float FloatSpeed = 2.f;

	UPROPERTY(EditAnywhere, Category = "Pickup|Animation")
	float RotationSpeed = 90.f;

	virtual void Interact(APlayerCharacter* Player) override;

	// Traces straight down onto the world floor (ECC_WorldStatic) near 'Around' and returns a point
	// resting on that surface, so dropped loot never spawns buried under the map. Falls back to
	// 'Around' unchanged if no floor is found. Mirrors the extraction-zone weapon placement.
	static FVector GroundedLocation(const UWorld* World, const FVector& Around, const AActor* IgnoreActor = nullptr);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* SphereCollision;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;

	float BaseZ = 0.f;
	float PhaseOffset = 0.f;
};
