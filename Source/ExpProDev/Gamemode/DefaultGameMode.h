// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DefaultGameMode.generated.h"

/**
 *  ONLY EXISTS ON THE SERVER.
 */
UCLASS()
class EXPPRODEV_API ADefaultGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class APlayerCharacter* EliminatedCharacter, class ADefaultPlayerController* VictimController, ADefaultPlayerController* AttackerController);
 // we may want to override in a child gamemode class so do a virtual
	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, class AController* EliminatedController);
};
