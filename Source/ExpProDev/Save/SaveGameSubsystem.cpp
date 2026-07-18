// No Rights Reserved @ Team Expedition

#include "Save/SaveGameSubsystem.h"
#include "Save/ExpProSaveGame.h"
#include "Kismet/GameplayStatics.h"

USaveGameSubsystem* USaveGameSubsystem::Get(const UObject* WorldContext)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	return GI ? GI->GetSubsystem<USaveGameSubsystem>() : nullptr;
}

void USaveGameSubsystem::MigrateIfNeeded(UExpProSaveGame* Save)
{
	if (!Save || Save->SaveVersion >= UExpProSaveGame::CurrentSaveVersion) return;

	// Sequential migrations run here as the schema grows — add one block per version bump, e.g.
	//   if (Save->SaveVersion < 2) { /* initialise fields added in version 2 */ }
	// No migrations exist yet (CurrentSaveVersion is 1); older saves just get re-stamped.

	Save->SaveVersion = UExpProSaveGame::CurrentSaveVersion;
}

UExpProSaveGame* USaveGameSubsystem::LoadActiveSlot(const UObject* WorldContext)
{
	USaveGameSubsystem* Sub = Get(WorldContext);
	if (!Sub) return nullptr;

	const FString Slot = Sub->GetActiveSlotName();
	if (!UGameplayStatics::DoesSaveGameExist(Slot, 0)) return nullptr;

	UExpProSaveGame* Save = Cast<UExpProSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
	MigrateIfNeeded(Save);
	return Save;
}

void USaveGameSubsystem::SaveToActiveSlot(const UObject* WorldContext, UExpProSaveGame* Save)
{
	if (!Save) return;

	USaveGameSubsystem* Sub = Get(WorldContext);
	if (!Sub) return;

	UGameplayStatics::SaveGameToSlot(Save, Sub->GetActiveSlotName(), 0);
}

bool USaveGameSubsystem::MutateActiveSlot(const UObject* WorldContext, TFunctionRef<void(UExpProSaveGame&)> Mutator)
{
	if (!Get(WorldContext)) return false;

	UExpProSaveGame* Save = LoadActiveSlot(WorldContext);
	if (!Save)
	{
		Save = Cast<UExpProSaveGame>(UGameplayStatics::CreateSaveGameObject(UExpProSaveGame::StaticClass()));
		if (!Save) return false;
		Save->SaveVersion = UExpProSaveGame::CurrentSaveVersion;
	}

	Mutator(*Save);
	SaveToActiveSlot(WorldContext, Save);
	return true;
}
