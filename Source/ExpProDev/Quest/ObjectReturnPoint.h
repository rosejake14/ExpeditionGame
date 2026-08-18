// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectReturnPoint.generated.h"

class UQuestDefinition;

// TECH_DEBT(TD-DEAD-6): DORMANT. bEnabled defaults to false and quest turn-in moved to
// AQuestGiverNPC, so every overlap early-outs. Kept only in case a placed instance still exists in
// a shipped map — verify references in-editor, then delete.
UCLASS()
class EXPPRODEV_API AObjectReturnPoint : public AActor
{
	GENERATED_BODY()
public:
	AObjectReturnPoint();

	// If set, only completes this specific quest. If null, completes any active quest with item collected.
	UPROPERTY(EditAnywhere, Category = "Quest")
	TObjectPtr<UQuestDefinition> LinkedQuest;

	// Disable the return-point trigger — quest completion now happens at the Quest Giver NPC
	UPROPERTY(EditAnywhere, Category = "Quest")
	bool bEnabled = false;

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
