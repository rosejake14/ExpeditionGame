// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "QuestGiverNPC.generated.h"

class UQuestDefinition;

UCLASS()
class EXPPRODEV_API AQuestGiverNPC : public AActor, public IInteractable
{
	GENERATED_BODY()
public:
	AQuestGiverNPC();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<TObjectPtr<UQuestDefinition>> AvailableQuests;

	virtual void Interact(APlayerCharacter* Player) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* InteractRadius;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
};
