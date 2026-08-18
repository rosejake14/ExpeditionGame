// No Rights Reserved @ Team Expedition 


#include "Gamemode/DefaultGameMode.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/DefaultPlayerController.h"

// TECH_DEBT(TD-DEAD-13): VictimController and AttackerController are accepted and then ignored.
// Every caller computes and passes them, so the death path looks like it supports kill attribution,
// scoring and death messages when it does none of that. Either use them or drop the parameters.
void ADefaultGameMode::PlayerEliminated(class APlayerCharacter* EliminatedCharacter,
	class ADefaultPlayerController* VictimController, ADefaultPlayerController* AttackerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminated();
	}
}

void ADefaultGameMode::RequestRespawn(class ACharacter* EliminatedCharacter, class AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if (EliminatedController)
	{
		TArray<AActor*> PlayerStartActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartActors);
		if (PlayerStartActors.Num() > 0)
		{
			const int32 StartSelection = FMath::RandRange(0, PlayerStartActors.Num() - 1);
			RestartPlayerAtPlayerStart(EliminatedController, PlayerStartActors[StartSelection]);
		}
		else
		{
			// No PlayerStarts on this map — fall back to the default spawn transform so we don't crash.
			UE_LOG(LogTemp, Warning, TEXT("RequestRespawn: no PlayerStart actors found; restarting at default transform."));
			RestartPlayer(EliminatedController);
		}
	}
}
