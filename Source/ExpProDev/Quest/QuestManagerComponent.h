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

// TECH_DEBT(TD-ARCH-4): exactly ONE quest can be active at a time (a single FActiveQuestState).
// The whole accept/cancel/complete API is written around that assumption, and AQuestGiverNPC has
// to block every other NPC while a quest is held. Supporting concurrent quests means reworking
// this API surface, the giver logic and the HUD together.
//
// TECH_DEBT(TD-ARCH-5): quest behaviour is a hardcoded EQuestType switch. Adding a third type
// (escort, reach-location, timed) means editing this component, UQuestDefinition and the HUD.
// Polymorphic objective objects on the definition would make it data-driven.
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

	// TECH_DEBT(TD-ARCH-14): completion history lives only on this component and is never persisted,
	// so every completed quest is offered again after a restart and can be farmed for XP.
	UPROPERTY()
	TSet<TObjectPtr<UQuestDefinition>> CompletedQuests;

	UPROPERTY()
	TObjectPtr<class ADefaultPlayerController> PlayerController;

	void OnInventoryChanged();
	void UpdateHUD();
	ADefaultPlayerController* GetPC();
};
