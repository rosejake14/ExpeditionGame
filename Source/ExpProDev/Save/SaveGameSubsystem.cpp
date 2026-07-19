// No Rights Reserved @ Team Expedition

#include "Save/SaveGameSubsystem.h"
#include "Save/ExpProSaveGame.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

USaveGameSubsystem* USaveGameSubsystem::Get(const UObject* WorldContext)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	return GI ? GI->GetSubsystem<USaveGameSubsystem>() : nullptr;
}

void USaveGameSubsystem::Deinitialize()
{
	// Shutting down: no more async writes, cancel the timer, and flush the latest state synchronously.
	bShuttingDown = true;
	if (UGameInstance* GI = GetGameInstance())
		GI->GetTimerManager().ClearTimer(DeferredFlushTimer);

	FlushSynchronous();

	Super::Deinitialize();
}

void USaveGameSubsystem::MigrateIfNeeded(UExpProSaveGame* Save)
{
	if (!Save || Save->SaveVersion >= UExpProSaveGame::CurrentSaveVersion) return;

	// Sequential migrations run here as the schema grows — add one block per version bump, e.g.
	//   if (Save->SaveVersion < 2) { /* initialise fields added in version 2 */ }
	// No migrations exist yet (CurrentSaveVersion is 1); older saves just get re-stamped.

	Save->SaveVersion = UExpProSaveGame::CurrentSaveVersion;
}

UExpProSaveGame* USaveGameSubsystem::GetOrLoadCached(bool bCreateIfMissing)
{
	// The active slot changed under us — flush and drop the stale cache before switching.
	if (CachedSave && CachedSlotIndex != ActiveSlotIndex)
	{
		FlushSynchronous();
		CachedSave = nullptr;
		CachedSlotIndex = INDEX_NONE;
	}

	if (!CachedSave)
	{
		const FString Slot = GetActiveSlotName();
		if (UGameplayStatics::DoesSaveGameExist(Slot, 0))
		{
			CachedSave = Cast<UExpProSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
			MigrateIfNeeded(CachedSave);
		}

		if (!CachedSave && bCreateIfMissing)
		{
			CachedSave = Cast<UExpProSaveGame>(UGameplayStatics::CreateSaveGameObject(UExpProSaveGame::StaticClass()));
			if (CachedSave)
				CachedSave->SaveVersion = UExpProSaveGame::CurrentSaveVersion;
		}

		if (CachedSave)
			CachedSlotIndex = ActiveSlotIndex;
	}

	return CachedSave;
}

UExpProSaveGame* USaveGameSubsystem::LoadActiveSlot(const UObject* WorldContext)
{
	USaveGameSubsystem* Sub = Get(WorldContext);
	return Sub ? Sub->GetOrLoadCached(/*bCreateIfMissing*/ false) : nullptr;
}

void USaveGameSubsystem::SaveToActiveSlot(const UObject* WorldContext, UExpProSaveGame* Save)
{
	if (!Save) return;

	USaveGameSubsystem* Sub = Get(WorldContext);
	if (!Sub) return;

	UGameplayStatics::SaveGameToSlot(Save, Sub->GetActiveSlotName(), 0);
}

bool USaveGameSubsystem::MutateActiveSlot(const UObject* WorldContext, TFunctionRef<void(UExpProSaveGame&)> Mutator,
	ESaveFlushPolicy Policy)
{
	USaveGameSubsystem* Sub = Get(WorldContext);
	if (!Sub) return false;

	UExpProSaveGame* Save = Sub->GetOrLoadCached(/*bCreateIfMissing*/ true);
	if (!Save) return false;

	Mutator(*Save);
	Sub->bDirty = true;

	if (Policy == ESaveFlushPolicy::Checkpoint)
		Sub->FlushAsync();
	else
		Sub->EnsureDeferredTimer();

	return true;
}

void USaveGameSubsystem::FlushNow(const UObject* WorldContext)
{
	if (USaveGameSubsystem* Sub = Get(WorldContext))
		Sub->FlushSynchronous();
}

void USaveGameSubsystem::DiscardCache(const UObject* WorldContext)
{
	USaveGameSubsystem* Sub = Get(WorldContext);
	if (!Sub) return;

	if (UGameInstance* GI = Sub->GetGameInstance())
		GI->GetTimerManager().ClearTimer(Sub->DeferredFlushTimer);

	Sub->CachedSave = nullptr;
	Sub->CachedSlotIndex = INDEX_NONE;
	Sub->bDirty = false;
	Sub->bFlushQueued = false;
}

void USaveGameSubsystem::FlushAsync()
{
	if (!CachedSave || !bDirty) return;

	// During shutdown async writes may be dropped — write straight to disk instead.
	if (bShuttingDown)
	{
		FlushSynchronous();
		return;
	}

	// Only one write to the slot at a time; if one is mid-flight, remember to flush again after.
	if (bSaveInFlight)
	{
		bFlushQueued = true;
		return;
	}

	// Snapshot the cache so gameplay can keep mutating CachedSave while the background thread writes.
	InFlightSave = DuplicateObject<UExpProSaveGame>(CachedSave, this);
	if (!InFlightSave) return;

	bSaveInFlight = true;
	bDirty = false;

	FAsyncSaveGameToSlotDelegate Delegate;
	Delegate.BindUObject(this, &USaveGameSubsystem::OnAsyncSaveComplete);
	UGameplayStatics::AsyncSaveGameToSlot(InFlightSave, GetActiveSlotName(), 0, Delegate);
}

void USaveGameSubsystem::FlushSynchronous()
{
	if (!CachedSave) return;

	UGameplayStatics::SaveGameToSlot(CachedSave, GetActiveSlotName(), 0);
	bDirty = false;
	bFlushQueued = false;
}

void USaveGameSubsystem::EnsureDeferredTimer()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	FTimerManager& TimerManager = GI->GetTimerManager();
	if (!TimerManager.IsTimerActive(DeferredFlushTimer))
	{
		TimerManager.SetTimer(DeferredFlushTimer, this,
			&USaveGameSubsystem::OnDeferredFlushTimer, DeferredFlushSeconds, /*bLoop*/ false);
	}
}

void USaveGameSubsystem::OnDeferredFlushTimer()
{
	if (bDirty)
		FlushAsync();
}

void USaveGameSubsystem::OnAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
	bSaveInFlight = false;
	InFlightSave = nullptr;

	if (!bSuccess)
		UE_LOG(LogTemp, Warning, TEXT("USaveGameSubsystem: async save to '%s' failed."), *SlotName);

	// Mutations that landed during the write need a follow-up flush (latest cache state wins).
	if ((bFlushQueued || bDirty) && !bShuttingDown)
	{
		bFlushQueued = false;
		FlushAsync();
	}
}
