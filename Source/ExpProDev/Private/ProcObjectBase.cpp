// No Rights Reserved @ Team Expedition 


#include "ProcObjectBase.h"

// Sets default values
AProcObjectBase::AProcObjectBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AProcObjectBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProcObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

