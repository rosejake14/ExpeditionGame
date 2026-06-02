// No Rights Reserved @ Team Expedition 


#include "ProcEnvironmentGenerator.h"
#include "ProceduralMeshComponent.h"
#include "ProcObjectBase.h"


// Sets default values
AProcEnvironmentGenerator::AProcEnvironmentGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
}

// Called when the game starts or when spawned
void AProcEnvironmentGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProcEnvironmentGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProcEnvironmentGenerator::GenProceduralGround()
{
	ProcMesh_PosAdjust = FVector(-ProcMesh_Size / 2, -ProcMesh_Size / 2, -50);
	AActor::SetActorLocation(ProcMesh_PosAdjust);

	ProcMesh_Verts.SetNum(ProcMesh_EdgeVertNum * ProcMesh_EdgeVertNum);

	int currentVertIndex = 0;
	float randomPivot = FMath::RandRange(.8, 1.2);

	for (int i = 0; i < ProcMesh_EdgeVertNum; i++) // Vert placement verti
	{
		for (int j = 0; j < ProcMesh_EdgeVertNum; j++) // Vert placement hori
		{
			ProcMesh_Verts[currentVertIndex++] = FVector((ProcMesh_Size / ProcMesh_EdgeVertNum) * j, (ProcMesh_Size / ProcMesh_EdgeVertNum) * i, FMath::PerlinNoise2D(FVector2d((j * randomPivot) / ProcMesh_Frequency, (i * randomPivot) / ProcMesh_Frequency)) * ProcMesh_Amplitude);
		}
	}

	ProcMesh_Tris.SetNum(6 * FMath::Pow(ProcMesh_EdgeVertNum, static_cast<double>(2)));

	int currentTriIndex = 0;
	int currentBaseIndex = 0;

	for (int i = 0; i < ProcMesh_EdgeVertNum - 1; i++) // Vert placement verti
	{
		for (int j = 0; j < ProcMesh_EdgeVertNum - 1; j++) // Vert placement hori
		{
			ProcMesh_Tris[currentTriIndex++] = currentBaseIndex;
			ProcMesh_Tris[currentTriIndex++] = currentBaseIndex + ProcMesh_EdgeVertNum;
			ProcMesh_Tris[currentTriIndex++] = currentBaseIndex + ProcMesh_EdgeVertNum + 1;

			ProcMesh_Tris[currentTriIndex++] = currentBaseIndex;
			ProcMesh_Tris[currentTriIndex++] = currentBaseIndex + ProcMesh_EdgeVertNum + 1;
			ProcMesh_Tris[currentTriIndex++] = currentBaseIndex + 1;

			currentBaseIndex++;
		}
		currentBaseIndex++;
	}

	ProcMesh->CreateMeshSection_LinearColor(0, ProcMesh_Verts, ProcMesh_Tris, TArray<FVector>(), TArray<FVector2D>(), ProcMesh_VertColors, TArray<FProcMeshTangent>(), true);


}

void AProcEnvironmentGenerator::GenProceduralObjects(TSubclassOf<class AProcObjectBase> ObjectToSpawn, float SpawnFrequency, bool RotateObjectToTerrain, bool VaryObjectScale)
{
	float randomPivot = FMath::RandRange(.1, .8);

	for (int i = 0; i < ProcMesh_Verts.Num(); i++)
	{
		float perlinVal = FMath::PerlinNoise2D(FVector2d(ProcMesh_Verts[i].X * randomPivot, ProcMesh_Verts[i].Y * randomPivot));
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%f"), FMath::Lerp(-1.0, 1.0, SpawnFrequency)));

		if (perlinVal <= FMath::Lerp(-1.0, 1.0, SpawnFrequency))
		{
			FVector randomNoise = FVector(FMath::RandRange(0.0, (ProcMesh_Size / ProcMesh_EdgeVertNum) / 1.0), FMath::RandRange(0.0, (ProcMesh_Size / ProcMesh_EdgeVertNum) / 1.0), 0.0);

			AProcObjectBase* newObj = GetWorld()->SpawnActor<AProcObjectBase>(ObjectToSpawn, ProcMesh_Verts[i] + ProcMesh_PosAdjust + randomNoise, FRotator());

			//newObj->RotateToTerrain = true;

			//newObj->RotateToTerrain = RotateObjectToTerrain;
			//newObj->VaryScale = VaryObjectScale;


		}
	}
}

void AProcEnvironmentGenerator::RegenProceduralGround()
{
	ProcMesh_Verts.SetNum(ProcMesh_EdgeVertNum * ProcMesh_EdgeVertNum);

	int currentVertIndex = 0;
	float randomPivot = FMath::RandRange(.8, 1.2);

	for (int i = 0; i < ProcMesh_EdgeVertNum; i++) // Vert placement verti
	{
		for (int j = 0; j < ProcMesh_EdgeVertNum; j++) // Vert placement hori
		{
			ProcMesh_Verts[currentVertIndex++] = FVector((ProcMesh_Size / ProcMesh_EdgeVertNum) * j, (ProcMesh_Size / ProcMesh_EdgeVertNum) * i, FMath::PerlinNoise2D(FVector2d((j * randomPivot) / ProcMesh_Frequency, (i * randomPivot) / ProcMesh_Frequency)) * ProcMesh_Amplitude);
		}
	}

	ProcMesh->UpdateMeshSection_LinearColor(0, ProcMesh_Verts, TArray<FVector>(), TArray<FVector2D>(), ProcMesh_VertColors, TArray<FProcMeshTangent>());
}