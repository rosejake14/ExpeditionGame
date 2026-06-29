// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Extraction/ExtractionTypes.h"
#include "SellRowWidget.generated.h"

UCLASS()
class EXPPRODEV_API USellRowWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetEntry(const FSellEntry& Entry);

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* ItemIconImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* QuantityText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CoinsEarnedText;
};
