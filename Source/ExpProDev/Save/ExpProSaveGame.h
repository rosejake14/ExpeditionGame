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
	UPROPERTY(BlueprintReadOnly)
	float XP = 0.f;

	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 DOSCoins = 0;

	UPROPERTY()
	TMap<FName, int32> PurchasedUpgrades;

	// WeaponId -> quantity owned. Consumable: decremented when a weapon is picked up in the world.
	UPROPERTY()
	TMap<FName, int32> PurchasedWeapons;
};
