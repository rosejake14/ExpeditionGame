// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Extraction/ExtractionTypes.h"
#include "PlayerHUD.generated.h"

class UInventoryComponent;
class UInventoryScreenWidget;
class UQuestWidget;
class UQuestSelectionWidget;
class UQuestDefinition;
class APlayerCharacter;
class AQuestGiverNPC;
class USellSummaryWidget;
class UExtractPromptWidget;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	// UPROPERTY so the crosshair textures are GC-tracked while held in the package.
	UPROPERTY()
	class UTexture2D* CrosshairCentre = nullptr;
	UPROPERTY()
	UTexture2D* CrosshairTop = nullptr;
	UPROPERTY()
	UTexture2D* CrosshairBottom = nullptr;
	UPROPERTY()
	UTexture2D* CrosshairRight = nullptr;
	UPROPERTY()
	UTexture2D* CrosshairLeft = nullptr;
};


/**
 * 
 */
UCLASS()
class EXPPRODEV_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = PlayerStats)
	TSubclassOf<class UUserWidget> PlayerOverlayClass;
	UPROPERTY()
	class UPlayerOverlay* PlayerOverlay;

	UPROPERTY(EditAnywhere, Category = PlayerStats)
	TSubclassOf<class UHotbarWidget> HotbarWidgetClass;
	UPROPERTY()
	class UHotbarWidget* HotbarWidget;

	UPROPERTY(EditAnywhere, Category = PlayerStats)
	TSubclassOf<UInventoryScreenWidget> InventoryScreenWidgetClass;
	UPROPERTY()
	UInventoryScreenWidget* InventoryScreenWidget;

	UPROPERTY(EditAnywhere, Category = "Quest")
	TSubclassOf<UQuestWidget> QuestWidgetClass;
	UPROPERTY()
	UQuestWidget* QuestWidget;

	UPROPERTY(EditAnywhere, Category = "Quest")
	TSubclassOf<UQuestSelectionWidget> QuestSelectionWidgetClass;
	UPROPERTY()
	UQuestSelectionWidget* QuestSelectionWidget;

	UPROPERTY(EditAnywhere, Category = "Extraction")
	TSubclassOf<USellSummaryWidget> SellSummaryWidgetClass;
	UPROPERTY()
	USellSummaryWidget* SellSummaryWidget;

	UPROPERTY(EditAnywhere, Category = "Extraction")
	TSubclassOf<UExtractPromptWidget> ExtractPromptWidgetClass;
	UPROPERTY()
	UExtractPromptWidget* ExtractPromptWidget;

	void ShowQuestSelection(APlayerCharacter* Player, const TArray<UQuestDefinition*>& Quests, AQuestGiverNPC* Giver);
	void ShowSellSummary(const TArray<FSellEntry>& Entries, int32 TotalEarned, int32 NewBalance);

	// Extraction-zone prompt ("Press [X] to Extract"). PromptText is resolved by the caller.
	void ShowExtractPrompt(const FText& PromptText);
	void HideExtractPrompt();

protected:
	virtual void BeginPlay() override;
	void AddPlayerOverlay();
	void AddHotbarWidget();
	void AddInventoryScreen();
	void AddQuestWidget();
private:
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* TextureToDraw, FVector2D ViewportCentre);

public:
	// Called from DefaultPlayerController::OnPossess — safe to call before or after HUD BeginPlay
	void InitHotbarForInventory(class UInventoryComponent* Inventory);
	void InitInventoryScreenForInventory(class UInventoryComponent* Inventory);

	// Called from PlayerCharacter input — toggles the full inventory screen and cursor
	void ToggleInventoryScreen();

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
