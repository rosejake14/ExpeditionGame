// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

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
};
