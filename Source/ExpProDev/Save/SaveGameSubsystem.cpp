// No Rights Reserved @ Team Expedition

#include "Save/SaveGameSubsystem.h"
#include "Save/ExpProSaveGame.h"
#include "Kismet/GameplayStatics.h"

UExpProSaveGame* USaveGameSubsystem::LoadActiveSlot(const UObject* WorldContext)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	if (!GI) return nullptr;

	USaveGameSubsystem* Sub = GI->GetSubsystem<USaveGameSubsystem>();
	if (!Sub) return nullptr;

	const FString Slot = Sub->GetActiveSlotName();
	if (!UGameplayStatics::DoesSaveGameExist(Slot, 0)) return nullptr;

	return Cast<UExpProSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
}

void USaveGameSubsystem::SaveToActiveSlot(const UObject* WorldContext, UExpProSaveGame* Save)
{
	if (!Save) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	if (!GI) return;

	USaveGameSubsystem* Sub = GI->GetSubsystem<USaveGameSubsystem>();
	if (!Sub) return;

	UGameplayStatics::SaveGameToSlot(Save, Sub->GetActiveSlotName(), 0);
}
