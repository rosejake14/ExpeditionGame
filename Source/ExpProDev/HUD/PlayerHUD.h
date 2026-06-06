// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

class UInventoryComponent;
class UInventoryScreenWidget;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairCentre;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairLeft;
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

protected:
	virtual void BeginPlay() override;
	void AddPlayerOverlay();
	void AddHotbarWidget();
	void AddInventoryScreen();
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
