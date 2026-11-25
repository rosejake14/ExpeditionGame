// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ExpProGameInstance.generated.h"

/**
 *  The project's GameInstance — the one object that survives a map change.
 *
 *  It exists for two reasons:
 *
 *   1. It holds the player's pre-run choices (which island, which hero) across the
 *      menu -> island travel. The menu widgets write them; ADefaultGameMode reads them on the
 *      far side to decide which pawn to spawn. Nothing else in the project can carry state
 *      across a level load.
 *
 *   2. It initialises the Gameplay Ability System exactly once, at startup. GAS does not fail
 *      loudly when InitGlobalData() is skipped — it crashes much later, the first time an
 *      ability uses target data. Doing it here means it cannot be forgotten.
 *
 *  Registered as GameInstanceClass in Config/DefaultEngine.ini. Resolve it from anywhere with
 *  Get(), following the same static-resolver pattern as UEconomySubsystem and USaveGameSubsystem.
 */
UCLASS()
class EXPPRODEV_API UExpProGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	/** Resolves the game instance from any world context. Null if unavailable or misconfigured. */
	static UExpProGameInstance* Get(const UObject* WorldContext);

	//
	// Pre-run selections.
	//
	// Nothing clears these — they persist until overwritten, so returning to the menu remembers
	// the last choice. Both are unset on a fresh boot and callers must handle that.
	//

	/** HeroId of the chosen UHeroDefinition. NAME_None until the hero select screen sets it. */
	UFUNCTION(BlueprintPure, Category = "Run")
	FName GetSelectedHeroId() const { return SelectedHeroId; }

	/** Records the chosen hero. Called by the hero select screen. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void SetSelectedHeroId(FName HeroId);

	/** Island the player picked. Null until the level picker sets it. */
	UFUNCTION(BlueprintPure, Category = "Run")
	TSoftObjectPtr<UWorld> GetSelectedLevel() const { return SelectedLevel; }

	/** Records the chosen island. Called by WBP_LevelPicker. */
	UFUNCTION(BlueprintCallable, Category = "Run")
	void SetSelectedLevel(TSoftObjectPtr<UWorld> Level);

	/**
	 *  Travels to the island stored in SelectedLevel. Returns false (and does nothing) when no
	 *  level has been chosen, so a misconfigured menu fails visibly in the log rather than
	 *  silently doing nothing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Run")
	bool OpenSelectedLevel();

private:
	UPROPERTY()
	FName SelectedHeroId;

	UPROPERTY()
	TSoftObjectPtr<UWorld> SelectedLevel;
};
