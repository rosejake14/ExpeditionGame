// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QuestDefinition.generated.h"

class UItemDefinition;

UCLASS(BlueprintType)
class EXPPRODEV_API UQuestDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
	FText CollectObjective = FText::FromString("Collect Item");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
	FText ReturnObjective = FText::FromString("Return Item");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
	FText CompleteText = FText::FromString("Quest Completed");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Item")
	TObjectPtr<UItemDefinition> RequiredItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Item", meta = (ClampMin = 1))
	int32 RequiredQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Rewards")
	float XPReward = 50.f;
};
