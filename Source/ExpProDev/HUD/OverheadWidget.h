// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class EXPPRODEV_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString TextToDisplay);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn, bool ShowPlayerName = true);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerRemoteRole(APawn* InPawn, bool ShowPlayerName = true);
	
	FString GetPlayerName(APlayerState* InPlayerState);

private:
	// virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override; // Called when leaving/chanigng world/level
	virtual void NativeDestruct() override;
private:
	
};
