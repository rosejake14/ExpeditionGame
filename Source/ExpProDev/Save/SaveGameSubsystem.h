// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class UExpProSaveGame;

UCLASS()
class EXPPRODEV_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// Set by the file-select screen; read by all save/load code throughout the game.
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 ActiveSlotIndex = 0;

	// Slot name for the currently active slot.
	UFUNCTION(BlueprintCallable, Category = "Save")
	FString GetActiveSlotName() const { return GetSlotName(ActiveSlotIndex); }

	// Slot name for any arbitrary index (used by the file-select screen to probe all slots).
	UFUNCTION(BlueprintCallable, Category = "Save")
	static FString GetSlotName(int32 Index) { return FString::Printf(TEXT("SaveFile_%d"), Index); }

	// Loads the save in the active slot (migrating it if it predates the current schema),
	// or returns nullptr if none exists.
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContext"))
	static UExpProSaveGame* LoadActiveSlot(const UObject* WorldContext);

	// Writes the given save object back to the active slot. This is the single wrapper around
	// SaveGameToSlot in the project — every other write goes through MutateActiveSlot.
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContext"))
	static void SaveToActiveSlot(const UObject* WorldContext, UExpProSaveGame* Save);

	// Loads (or creates) the active slot, applies Mutator to it, then writes it back.
	// This is the only supported way to modify a save: any field Mutator doesn't touch is
	// preserved, so no writer can silently clobber another system's data. Returns false only
	// if the active slot couldn't be resolved/created. Not a UFUNCTION (TFunctionRef isn't reflectable).
	static bool MutateActiveSlot(const UObject* WorldContext, TFunctionRef<void(UExpProSaveGame&)> Mutator);

private:
	// Resolves the save subsystem from any world context.
	static USaveGameSubsystem* Get(const UObject* WorldContext);

	// Applies version migrations in-place and re-stamps SaveVersion to current.
	static void MigrateIfNeeded(UExpProSaveGame* Save);
};
