// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectReturnPoint.generated.h"

class UQuestDefinition;

UCLASS()
class EXPPRODEV_API AObjectReturnPoint : public AActor
{
	GENERATED_BODY()
public:
	AObjectReturnPoint();

	// If set, only completes this specific quest. If null, completes any active quest with item collected.
	UPROPERTY(EditAnywhere, Category = "Quest")
	TObjectPtr<UQuestDefinition> LinkedQuest;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* SphereCollision;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
};
