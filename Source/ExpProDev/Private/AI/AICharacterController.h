// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICharacterController.generated.h"

// TECH_DEBT(TD-DEAD-3): DEAD CODE. The only thing that includes this header is DemoCharacterAI.cpp,
// which is itself dead (TD-DEAD-4). Delete both together, along with AAICharacter (TD-DEAD-2).
UCLASS()
class AAICharacterController : public AAIController
{
	GENERATED_BODY()
	
};
