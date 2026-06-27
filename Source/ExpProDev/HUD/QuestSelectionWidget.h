// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestSelectionWidget.generated.h"

class UQuestDefinition;
class APlayerCharacter;

UCLASS()
class EXPPRODEV_API UQuestSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Called by AQuestGiverNPC to populate and show the panel
	void InitQuestList(APlayerCharacter* InPlayer, const TArray<UQuestDefinition*>& Quests);

	// Call from Blueprint button OnClicked — accepts the quest and closes the panel
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SelectQuest(UQuestDefinition* Quest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClosePanel();

	// Blueprint iterates this to build the visual list
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TArray<TObjectPtr<UQuestDefinition>> QuestOptions;

protected:
	// Implement in Blueprint to rebuild the visual list after QuestOptions is populated
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	void OnQuestsReady();

private:
	UPROPERTY()
	TObjectPtr<APlayerCharacter> OwningPlayer;
};
