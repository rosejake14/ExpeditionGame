// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcObjectBase.generated.h"

UCLASS()
class EXPPRODEV_API AProcObjectBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcObjectBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	bool RotateToTerrain;

	UPROPERTY(BlueprintReadOnly)
	bool VaryScale;

	UPROPERTY(BlueprintReadWrite)
	bool Interactable = false;

};
