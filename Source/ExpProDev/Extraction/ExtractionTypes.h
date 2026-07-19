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

// Result of an extraction sell — returned by UEconomySubsystem::SellLoot so the pawn/UI
// can render the summary without owning the valuation logic.
USTRUCT(BlueprintType)
struct FSellResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FSellEntry> Entries;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalEarned = 0;

	// Player's DOS$ balance after the sale was credited.
	UPROPERTY(BlueprintReadOnly)
	int32 NewBalance = 0;
};
