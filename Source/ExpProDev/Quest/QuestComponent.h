// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EXPPRODEV_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UQuestComponent();
	virtual void BeginPlay() override;

	// The item that must be collected to progress the quest
	UPROPERTY(EditAnywhere, Category = "Quest")
	class UItemDefinition* RequiredItem = nullptr;

	bool IsItemCollected() const { return bItemCollected; }
	void NotifyQuestComplete();

private:
	bool bItemCollected = false;

	UPROPERTY()
	class ADefaultPlayerController* PlayerController;

	void OnInventoryChanged();
	void UpdateHUD();
};
