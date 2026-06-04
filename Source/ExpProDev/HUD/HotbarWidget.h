// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HotbarWidget.generated.h"

UCLASS()
class EXPPRODEV_API UHotbarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Called from DefaultPlayerController::OnPossess once both HUD and pawn exist
	void InitHotbar(class UInventoryComponent* Inventory);

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* SlotContainer;

	// Set this in the Blueprint to the WBP_InventorySlot blueprint class
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<class UInventorySlotWidget> SlotWidgetClass;

private:
	void Refresh();

	UPROPERTY()
	class UInventoryComponent* InventoryComp;

	UPROPERTY()
	TArray<class UInventorySlotWidget*> SlotWidgets;
};
