// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class EXPPRODEV_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

	ALobbyGameMode();

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumRequiredToStart = 2;
	// This gamemode only needs to be on Lobby NOT pre-lobby?!
};
