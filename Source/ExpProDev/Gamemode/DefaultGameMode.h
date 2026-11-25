// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DefaultGameMode.generated.h"

class APlayerCharacter;
class ADefaultPlayerController;

/** Broadcast once, at the moment the run clock crosses zero. The run does not end — see below. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRunTimerExpired);

/**
 *  ONLY EXISTS ON THE SERVER.
 *
 *  Owns the run clock. StartRunTimer counts down from RunDurationMinutes; on reaching zero it
 *  broadcasts OnRunTimerExpired exactly once and then keeps running into overtime, where
 *  GetRunTimeRemaining goes negative and climbs. Nothing stops it but StopRunTimer.
 *
 *  UPlayerOverlay polls GetRunTimeRemaining to draw the clock; OnRunTimerExpired is the hook a
 *  future boss spawn binds to.
 */
UCLASS()
class EXPPRODEV_API ADefaultGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	//
	// Hero selection.
	//

	/**
	 *  Every hero in the game. Set this to DA_HeroRegistry on BP_DefaultGameMode.
	 *  While it is unset the game mode falls back to DefaultPawnClass, so the project keeps
	 *  working exactly as it did before heroes existed.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heroes")
	class UHeroRegistry* HeroRegistry = nullptr;

	/**
	 *  The hero the player picked, or the registry's first entry when nothing was chosen —
	 *  which is the normal case when PIE starts straight on an island instead of via the menu.
	 *  Null when no registry is set.
	 */
	UFUNCTION(BlueprintPure, Category = "Heroes")
	class UHeroDefinition* ResolveSelectedHero() const;

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
		const FTransform& SpawnTransform) override;

	virtual void PlayerEliminated(class APlayerCharacter* EliminatedCharacter,
		class ADefaultPlayerController* VictimController, ADefaultPlayerController* AttackerController);
 // we may want to override in a child gamemode class so do a virtual
	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, class AController* EliminatedController);

	//
	// Run timer.
	//

	/** Length of the countdown, in minutes, before the clock hits zero and flips to overtime. */
	UPROPERTY(EditAnywhere, Category = "Run Timer", meta = (ClampMin = "0.0"))
	float RunDurationMinutes = 1.f;

	/** When true the clock starts on BeginPlay. Clear it on maps that have no timed run. */
	UPROPERTY(EditAnywhere, Category = "Run Timer")
	bool bStartRunTimerOnBeginPlay = true;

	/** Fires once as the clock crosses zero. Boss spawning will bind here. */
	UPROPERTY(BlueprintAssignable, Category = "Run Timer")
	FOnRunTimerExpired OnRunTimerExpired;

	/** (Re)starts the clock from RunDurationMinutes. Safe to call while one is already running. */
	UFUNCTION(BlueprintCallable, Category = "Run Timer")
	void StartRunTimer();

	/** Halts the clock. GetRunTimeRemaining returns 0 afterwards and the overlay hides the text. */
	UFUNCTION(BlueprintCallable, Category = "Run Timer")
	void StopRunTimer();

	/**
	 *  Signed seconds: > 0 while counting down, < 0 once in overtime (magnitude is how far past
	 *  zero the run is), and exactly 0 when no clock is running.
	 */
	UFUNCTION(BlueprintPure, Category = "Run Timer")
	float GetRunTimeRemaining() const;

	/** True from StartRunTimer until StopRunTimer — stays true throughout overtime. */
	UFUNCTION(BlueprintPure, Category = "Run Timer")
	bool IsRunTimerRunning() const { return bRunTimerRunning; }

	/** True once the clock has crossed zero and is counting upwards. */
	UFUNCTION(BlueprintPure, Category = "Run Timer")
	bool IsInOvertime() const;

protected:
	virtual void BeginPlay() override;

private:
	/** World time the current run started at. Pause-aware, so the clock freezes with the game. */
	double RunStartTimeSeconds = 0.0;

	bool bRunTimerRunning = false;

	/** One-shot, fires OnRunTimerFinished at zero. Deliberately not used to read the clock — it
	 *  cannot express overtime, so the display works off RunStartTimeSeconds instead. */
	FTimerHandle RunExpiryHandle;

	void OnRunTimerFinished();
};
