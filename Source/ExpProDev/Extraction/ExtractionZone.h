// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "ExtractionZone.generated.h"

class UWeaponRegistry;

UCLASS()
class EXPPRODEV_API AExtractionZone : public AActor, public IInteractable
{
	GENERATED_BODY()
public:
	AExtractionZone();

	virtual void Interact(APlayerCharacter* Player) override;

	// Set to DA_WeaponRegistry on BP_ExtractionZone — used to map saved weapon IDs to pickups.
	UPROPERTY(EditAnywhere, Category = "Extraction")
	UWeaponRegistry* WeaponRegistry;

	// How far up/down to trace when placing spawned weapons on the ground.
	UPROPERTY(EditAnywhere, Category = "Extraction", meta = (ClampMin = 0.f))
	float GroundTraceHeight = 2000.f;

protected:
	virtual void BeginPlay() override;

	// Spawns the player's purchased weapons inside the box; consumption happens on pickup.
	void SpawnPurchasedWeapons();

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxCollision;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Extraction")
	class UBillboardComponent* SpriteComponent;
#endif
};
