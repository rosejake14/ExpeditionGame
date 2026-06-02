// No Rights Reserved @ Team Expedition 


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "HUD/PlayerHUD.h"
#include "PlayerController/DefaultPlayerController.h"

ALobbyGameMode::ALobbyGameMode()
{
	PlayerControllerClass = ADefaultPlayerController::StaticClass();
	HUDClass = APlayerHUD::StaticClass();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
	if (NumOfPlayers == NumRequiredToStart)
	{
		if (GetWorld())
		{
			bUseSeamlessTravel = true;
			GetWorld()->ServerTravel(FString("/Game/Maps/Prototyping/LVL_Island_Greybox?listen"));
		}
	}
}
