// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestManagerComponent.generated.h"

class UQuestDefinition;

USTRUCT()
struct FActiveQuestState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UQuestDefinition> Definition;

	bool bItemCollected = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EXPPRODEV_API UQuestManagerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UQuestManagerComponent();
	virtual void BeginPlay() override;

	void AcceptQuest(UQuestDefinition* Quest);
	void NotifyQuestComplete(UQuestDefinition* Quest);

	bool HasActiveQuest() const { return ActiveQuest.Definition != nullptr; }
	bool IsItemCollectedFor(UQuestDefinition* Quest) const;
	UQuestDefinition* GetActiveQuestDefinition() const { return ActiveQuest.Definition; }

private:
	FActiveQuestState ActiveQuest;

	UPROPERTY()
	TObjectPtr<class ADefaultPlayerController> PlayerController;

	void OnInventoryChanged();
	void UpdateHUD();
	ADefaultPlayerController* GetPC();
};
