// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Extraction/ExtractionTypes.h"
#include "SellSummaryWidget.generated.h"

UCLASS()
class EXPPRODEV_API USellSummaryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "Extraction")
	TSubclassOf<class USellRowWidget> RowWidgetClass;

	void InitSummary(const TArray<FSellEntry>& Entries, int32 TotalEarned, int32 NewBalance);

protected:
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ItemListContainer;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TotalCoinsText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TotalBalanceText;

	// Wire OnClicked to OpenLevel in the Blueprint subclass
	UPROPERTY(meta = (BindWidget))
	class UButton* ContinueButton;
};
