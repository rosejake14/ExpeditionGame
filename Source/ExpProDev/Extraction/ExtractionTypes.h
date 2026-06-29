// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "ExtractionTypes.generated.h"

USTRUCT(BlueprintType)
struct FSellEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	class UTexture2D* Icon = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CoinsEarned = 0;
};
