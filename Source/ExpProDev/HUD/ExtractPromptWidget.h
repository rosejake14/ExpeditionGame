// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ExtractPromptWidget.generated.h"

// Small "Press [X] to Extract" prompt shown while the player stands in an extraction zone.
// The widget BP should contain a UTextBlock named 'PromptText'.
UCLASS()
class EXPPRODEV_API UExtractPromptWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetPromptText(const FText& Text);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* PromptText;
};
