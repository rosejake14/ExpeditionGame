// No Rights Reserved @ Team Expedition 


#include "HUD/PlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "PlayerOverlay.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && PlayerOverlayClass)
	{
		PlayerOverlay = CreateWidget<UPlayerOverlay>(PlayerController, PlayerOverlayClass);
	} 
	AddPlayerOverlay();
}

void APlayerHUD::AddPlayerOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && PlayerOverlayClass)
	{
		PlayerOverlay = CreateWidget<UPlayerOverlay>(PlayerController, PlayerOverlayClass);
		if (PlayerOverlay)
		{
			PlayerOverlay->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("PlayerHUD.cpp: PlayerOverlay created and added"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerHUD.cpp: Failed to create PlayerOverlay widget"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerHUD.cpp: PlayerOverlayClass or Controller missing"));
	}
}

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCentre(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);

		if (HUDPackage.CrosshairCentre)
		{
			DrawCrosshair(HUDPackage.CrosshairCentre, ViewportCentre);
		}
		if (HUDPackage.CrosshairBottom)
		{
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCentre);
		}
		if (HUDPackage.CrosshairLeft)
		{
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCentre);
		}
		if (HUDPackage.CrosshairRight)
		{
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCentre);
		}
		if (HUDPackage.CrosshairTop)
		{
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCentre);
		}
	}
}


void APlayerHUD::DrawCrosshair(UTexture2D* TextureToDraw, FVector2D ViewportCentre)
{
	const float TextureWidth = TextureToDraw->GetSizeX();
	const float TextureHeight = TextureToDraw->GetSizeY();
	// Centre of screen moved by the necassary amount based on the texture WxH
	const FVector2D TextureDrawPoint(
		ViewportCentre.X - (TextureWidth / 2.f),
		ViewportCentre.Y - (TextureHeight / 2.f)
		);

	DrawTexture(
		TextureToDraw,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White,
		BLEND_Translucent,
		1.f
		);
}
