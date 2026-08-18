// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestComponent.generated.h"

// TECH_DEBT(TD-DEAD-5): DEAD CODE. Fully superseded by UQuestManagerComponent — this component is
// added to nothing in C++ and implements a single hardcoded collect-and-return quest. It still
// binds UInventoryComponent::OnInventoryChanged and writes quest HUD text, so if a Blueprint ever
// adds it to the pawn it will fight UQuestManagerComponent for the same HUD slot. Delete it.

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
