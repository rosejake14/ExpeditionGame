// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class UExpProSaveGame;

// Controls how a mutation reaches disk.
UENUM()
enum class ESaveFlushPolicy : uint8
{
	// Gameplay milestone (purchase, sell, level-up, slot select) — flush asynchronously right away.
	Checkpoint,
	// High-frequency tick (coin pickup, XP gain) — mutate the cache and mark dirty; the write is
	// coalesced onto the 30s timer / the next checkpoint so we don't hitch on every event.
	Deferred
};

/**
 * Owns the active save slot in memory. Everything reads and writes through here so the game
 * touches disk at most once per checkpoint (async) and once on the 30s dirty timer, instead of
 * synchronously on every coin/XP tick. EndPlay / shutdown flush synchronously.
 */
UCLASS()
class EXPPRODEV_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Deinitialize() override;

	// Set by the file-select screen; read by all save/load code throughout the game.
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 ActiveSlotIndex = 0;

	// Slot name for the currently active slot.
	UFUNCTION(BlueprintCallable, Category = "Save")
	FString GetActiveSlotName() const { return GetSlotName(ActiveSlotIndex); }

	// Slot name for any arbitrary index (used by the file-select screen to probe all slots).
	UFUNCTION(BlueprintCallable, Category = "Save")
	static FString GetSlotName(int32 Index) { return FString::Printf(TEXT("SaveFile_%d"), Index); }

	// Returns the in-memory save for the active slot, loading it from disk on first access
	// (migrating older schemas). Returns nullptr only if the slot has never been written.
	// All in-game readers go through this so they see the freshest (pre-flush) state.
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContext"))
	static UExpProSaveGame* LoadActiveSlot(const UObject* WorldContext);

	// Synchronous write of an explicit save object to the active slot (menu / compatibility use).
	// TECH_DEBT(TD-DEAD-11): no callers, and it is a trap — it writes straight past CachedSave, so
	// the in-memory cache is left stale and the next flush silently overwrites whatever it wrote.
	// Remove it, or make it route through the cache like every other writer.
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContext"))
	static void SaveToActiveSlot(const UObject* WorldContext, UExpProSaveGame* Save);

	// Loads (or creates) the cached active slot, applies Mutator, then flushes per Policy.
	// Any field Mutator doesn't touch is preserved. Not a UFUNCTION (TFunctionRef isn't reflectable).
	static bool MutateActiveSlot(const UObject* WorldContext, TFunctionRef<void(UExpProSaveGame&)> Mutator,
		ESaveFlushPolicy Policy = ESaveFlushPolicy::Checkpoint);

	// Synchronously writes any dirty cached state now. Used on quit (APlayerCharacter::EndPlay) and
	// before a slot is deleted, where an async write could be dropped at shutdown.
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContext"))
	static void FlushNow(const UObject* WorldContext);

	// Drops the cached save without writing it (used by WipeSave after the slot is deleted on disk,
	// so a pending flush can't resurrect the file).
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContext"))
	static void DiscardCache(const UObject* WorldContext);

private:
	// Resolves the save subsystem from any world context.
	static USaveGameSubsystem* Get(const UObject* WorldContext);

	// Applies version migrations in-place and re-stamps SaveVersion to current.
	static void MigrateIfNeeded(UExpProSaveGame* Save);

	// Returns the cached save for the active slot, loading from disk / creating as needed. If the
	// active slot changed since we last cached, the old cache is flushed synchronously first.
	UExpProSaveGame* GetOrLoadCached(bool bCreateIfMissing);

	void FlushAsync();
	void FlushSynchronous();
	void EnsureDeferredTimer();
	void OnDeferredFlushTimer();
	void OnAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess);

	// The authoritative in-memory save for CachedSlotIndex.
	UPROPERTY() TObjectPtr<UExpProSaveGame> CachedSave = nullptr;

	// A GC-safe snapshot handed to the background writer so gameplay can keep mutating CachedSave.
	UPROPERTY() TObjectPtr<UExpProSaveGame> InFlightSave = nullptr;

	int32 CachedSlotIndex = INDEX_NONE;
	bool bDirty = false;
	bool bSaveInFlight = false;
	bool bFlushQueued = false;   // a flush was requested while one was already in flight
	bool bShuttingDown = false;  // during shutdown all writes go synchronous

	FTimerHandle DeferredFlushTimer;

	static constexpr float DeferredFlushSeconds = 30.f;
};
