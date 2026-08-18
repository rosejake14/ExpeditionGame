// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QuestDefinition.generated.h"

class UItemDefinition;

UENUM(BlueprintType)
enum class EQuestType : uint8
{
	ItemCollection      UMETA(DisplayName = "Item Collection"),
	EnemyElimination    UMETA(DisplayName = "Enemy Elimination"),
};

// TECH_DEBT(TD-ARCH-13): no stable FName Id — quests can't be written to a save slot, which is why
// active-quest progress and completion history are lost on quit (TD-ARCH-14).
UCLASS(BlueprintType)
class EXPPRODEV_API UQuestDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	EQuestType QuestType = EQuestType::ItemCollection;

	// ── Shared ────────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
	FText CompleteText = FText::FromString("Quest Completed");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Rewards")
	float XPReward = 50.f;

	// ── Item Collection ───────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Item Collection",
		meta = (EditCondition = "QuestType == EQuestType::ItemCollection", EditConditionHides))
	FText CollectObjective = FText::FromString("Collect Item");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Item Collection",
		meta = (EditCondition = "QuestType == EQuestType::ItemCollection", EditConditionHides))
	FText ReturnObjective = FText::FromString("Return Item");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Item Collection",
		meta = (EditCondition = "QuestType == EQuestType::ItemCollection", EditConditionHides))
	TObjectPtr<UItemDefinition> RequiredItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Item Collection",
		meta = (EditCondition = "QuestType == EQuestType::ItemCollection", EditConditionHides, ClampMin = 1))
	int32 RequiredQuantity = 1;

	// ── Enemy Elimination ─────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Enemy Elimination",
		meta = (EditCondition = "QuestType == EQuestType::EnemyElimination", EditConditionHides))
	FText EliminationObjective = FText::FromString("Eliminate Enemies");

	/** Which enemy Blueprint to count. Leave empty to count any enemy kill. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Enemy Elimination",
		meta = (EditCondition = "QuestType == EQuestType::EnemyElimination", EditConditionHides))
	TSubclassOf<ACharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Enemy Elimination",
		meta = (EditCondition = "QuestType == EQuestType::EnemyElimination", EditConditionHides, ClampMin = 1))
	int32 KillsRequired = 5;
};
