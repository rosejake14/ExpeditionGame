// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DefaultPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class EXPPRODEV_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetHUDHealth(float Health, float MaxHealth);

	void SetHUDKillCount(int32 KillCount);
	void AddKill();

	void SetHUDXP(float XP, float XPToNextLevel, int32 Level);

	virtual void OnPossess(APawn* InPawn) override;
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	class APlayerHUD* PlayerHUD;

	int32 KillCount = 0;
};
