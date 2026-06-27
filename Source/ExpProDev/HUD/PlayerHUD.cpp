// No Rights Reserved @ Team Expedition 


#include "HUD/PlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "PlayerOverlay.h"
#include "HotbarWidget.h"
#include "InventoryScreenWidget.h"
#include "QuestWidget.h"
#include "QuestSelectionWidget.h"
#include "Character/PlayerCharacter.h"
#include "Quest/QuestDefinition.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();
	AddPlayerOverlay();
	AddHotbarWidget();
	AddInventoryScreen();
	AddQuestWidget();
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

void APlayerHUD::AddHotbarWidget()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !HotbarWidgetClass) return;

	HotbarWidget = CreateWidget<UHotbarWidget>(PC, HotbarWidgetClass);
	if (!HotbarWidget) return;

	HotbarWidget->AddToViewport();

	// Scenario B: OnPossess already fired before HUD BeginPlay — pawn is available now
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
	if (PlayerChar)
	{
		HotbarWidget->InitHotbar(PlayerChar->GetInventory());
	}
}

void APlayerHUD::InitHotbarForInventory(UInventoryComponent* Inventory)
{
	if (HotbarWidget)
	{
		HotbarWidget->InitHotbar(Inventory);
	}
}

void APlayerHUD::AddInventoryScreen()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	// Fall back to the C++ class if no Blueprint is assigned — no WBP_InventoryScreen required
	TSubclassOf<UInventoryScreenWidget> ClassToUse = InventoryScreenWidgetClass
		? InventoryScreenWidgetClass
		: TSubclassOf<UInventoryScreenWidget>(UInventoryScreenWidget::StaticClass());

	InventoryScreenWidget = CreateWidget<UInventoryScreenWidget>(PC, ClassToUse);
	if (!InventoryScreenWidget) return;

	InventoryScreenWidget->AddToViewport(1); // zorder 1 so it renders above hotbar

	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
	if (PlayerChar)
	{
		InventoryScreenWidget->InitInventory(PlayerChar->GetInventory());
	}
}

void APlayerHUD::InitInventoryScreenForInventory(UInventoryComponent* Inventory)
{
	if (InventoryScreenWidget)
	{
		InventoryScreenWidget->InitInventory(Inventory);
	}
}

void APlayerHUD::ToggleInventoryScreen()
{
	if (!InventoryScreenWidget) return;

	InventoryScreenWidget->ToggleScreen();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	const bool bIsOpen = InventoryScreenWidget->GetVisibility() == ESlateVisibility::Visible;
	PC->SetShowMouseCursor(bIsOpen);
	if (bIsOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void APlayerHUD::AddQuestWidget()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !QuestWidgetClass) return;

	QuestWidget = CreateWidget<UQuestWidget>(PC, QuestWidgetClass);
	if (QuestWidget)
		QuestWidget->AddToViewport();

	if (QuestSelectionWidgetClass)
	{
		QuestSelectionWidget = CreateWidget<UQuestSelectionWidget>(PC, QuestSelectionWidgetClass);
		if (QuestSelectionWidget)
		{
			QuestSelectionWidget->AddToViewport(2);
			QuestSelectionWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void APlayerHUD::ShowQuestSelection(APlayerCharacter* Player, const TArray<UQuestDefinition*>& Quests, AQuestGiverNPC* Giver)
{
	if (!QuestSelectionWidget) return;
	QuestSelectionWidget->InitQuestList(Player, Quests, Giver);
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
