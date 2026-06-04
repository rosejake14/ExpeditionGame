// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemPickup.generated.h"

UCLASS()
class EXPPRODEV_API AItemPickup : public AActor
{
	GENERATED_BODY()
public:
	AItemPickup();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	class UItemDefinition* ItemDef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = 1))
	int32 Quantity = 1;

protected:
	virtual void BeginPlay() override;

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
};
