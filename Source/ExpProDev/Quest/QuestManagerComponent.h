// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestManagerComponent.generated.h"

class UQuestDefinition;
class AQuestGiverNPC;

USTRUCT()
struct FActiveQuestState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UQuestDefinition> Definition;

	UPROPERTY()
	TObjectPtr<AQuestGiverNPC> QuestGiver;

	bool bItemCollected     = false;
	bool bObjectiveComplete = false;
	int32 KillCount         = 0;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EXPPRODEV_API UQuestManagerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UQuestManagerComponent();
	virtual void BeginPlay() override;

	void AcceptQuest(UQuestDefinition* Quest, AQuestGiverNPC* Giver);
	void NotifyQuestComplete(UQuestDefinition* Quest);
	void NotifyEnemyKilled(TSubclassOf<ACharacter> EnemyClass);

	// Returns true if the player had a completed quest from this NPC and rewards were granted
	bool TryCompleteQuestFromNPC(AQuestGiverNPC* NPC);

	void CancelActiveQuest();

	bool HasActiveQuest() const { return ActiveQuest.Definition != nullptr; }
	bool HasActiveQuestFrom(AQuestGiverNPC* NPC) const;
	bool HasCompletedQuest(UQuestDefinition* Quest) const { return CompletedQuests.Contains(Quest); }
	bool IsItemCollectedFor(UQuestDefinition* Quest) const;
	UQuestDefinition* GetActiveQuestDefinition() const { return ActiveQuest.Definition; }

private:
	FActiveQuestState ActiveQuest;

	UPROPERTY()
	TSet<TObjectPtr<UQuestDefinition>> CompletedQuests;

	UPROPERTY()
	TObjectPtr<class ADefaultPlayerController> PlayerController;

	void OnInventoryChanged();
	void UpdateHUD();
	ADefaultPlayerController* GetPC();
};
