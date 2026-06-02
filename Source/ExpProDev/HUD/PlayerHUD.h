// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

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

protected:
	virtual void BeginPlay() override;
	void AddPlayerOverlay();
private:
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* TextureToDraw, FVector2D ViewportCentre);

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
