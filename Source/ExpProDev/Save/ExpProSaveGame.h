// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ExpProSaveGame.generated.h"

UCLASS()
class EXPPRODEV_API UExpProSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY()
	float XP = 0.f;

	UPROPERTY()
	int32 Level = 1;

	UPROPERTY()
	int32 DOSCoins = 0;
};
