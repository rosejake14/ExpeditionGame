// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ProcEnvironmentGenerator.generated.h"

UCLASS()
class EXPPRODEV_API AProcEnvironmentGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcEnvironmentGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void GenProceduralGround();

	UFUNCTION(BlueprintCallable)
	void GenProceduralObjects(TSubclassOf<class AProcObjectBase> ObjectToSpawn, float SpawnFrequency, bool RotateObjectToTerrain, bool VaryObjectScal);

	UFUNCTION(BlueprintCallable)
	void RegenProceduralGround();

	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* ProcMesh;
	FVector ProcMesh_PosAdjust;

	UPROPERTY(EditAnywhere) int ProcMesh_EdgeVertNum;
	UPROPERTY(EditAnywhere) float ProcMesh_Size;
	UPROPERTY(EditAnywhere) float ProcMesh_Amplitude;
	UPROPERTY(EditAnywhere) float ProcMesh_Frequency;

	UPROPERTY(Meta = (MakeEditWidget = true))
	TArray<FVector> ProcMesh_Verts;

	TArray<int> ProcMesh_Tris;

	TArray<FLinearColor> ProcMesh_VertColors;

};
