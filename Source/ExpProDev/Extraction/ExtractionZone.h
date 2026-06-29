// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "ExtractionZone.generated.h"

UCLASS()
class EXPPRODEV_API AExtractionZone : public AActor, public IInteractable
{
	GENERATED_BODY()
public:
	AExtractionZone();

	virtual void Interact(APlayerCharacter* Player) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxCollision;
};
