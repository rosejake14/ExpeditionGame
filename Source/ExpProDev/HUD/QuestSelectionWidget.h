// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestSelectionWidget.generated.h"

class UQuestDefinition;
class APlayerCharacter;
class AQuestGiverNPC;

UCLASS()
class EXPPRODEV_API UQuestSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Called by AQuestGiverNPC to populate and show the panel
	void InitQuestList(APlayerCharacter* InPlayer, const TArray<UQuestDefinition*>& Quests, AQuestGiverNPC* InGiver);

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
	// TECH_DEBT(TD-BP-1): this event IS the quest list. With no C++ fallback, an unimplemented BP
	// means talking to a quest giver opens an empty panel that captures the mouse — the quest flow
	// looks broken with nothing logged to explain why.
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	void OnQuestsReady();

private:
	UPROPERTY()
	TObjectPtr<APlayerCharacter> OwningPlayer;

	UPROPERTY()
	TObjectPtr<AQuestGiverNPC> QuestGiver;
};
