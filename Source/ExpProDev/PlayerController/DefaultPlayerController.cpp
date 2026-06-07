// No Rights Reserved @ Team Expedition 


#include "DefaultPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/PlayerOverlay.h"
#include "HUD/PlayerHUD.h"
#include "HUD/HotbarWidget.h"
#include "Character/PlayerCharacter.h"

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD());
}


void ADefaultPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	// Is it valid, if not, cast to playerHUD.
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	UE_LOG(LogTemp, Warning, TEXT("Attempting to set hud health"));

	// Check in the order in which things are put together, checking the parent first, then the children.
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
			PlayerHUD->PlayerOverlay->HealthBar &&
				PlayerHUD->PlayerOverlay->HealthText;
	
	if (bHUDValid)
	{
		// Set Health Bar
		const float HealthPercent = Health / MaxHealth;
		PlayerHUD->PlayerOverlay->HealthBar->SetPercent(HealthPercent);
		// Set Health Text
		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		PlayerHUD->PlayerOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		if (PlayerHUD != nullptr)
		{
			if (PlayerHUD->PlayerOverlay != nullptr)
			{
				if (PlayerHUD->PlayerOverlay->HealthText != nullptr || PlayerHUD->PlayerOverlay->HealthBar != nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("Health Bar/Text Valid"));
				}else
				{
					UE_LOG(LogTemp, Warning, TEXT("Health Bar/Text InValid"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HUD Overlay Invalid"));
			}
		}else
		{
			UE_LOG(LogTemp, Warning, TEXT("HUD Invalid"));
		}
	}
	
}

//
// Updating HUD for the Kill Count.
//
void ADefaultPlayerController::SetHUDKillCount(int32 Count)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
			PlayerHUD->PlayerOverlay->KillCountText;

	if (bHUDValid)
	{
		FString KillText = FString::Printf(TEXT("Kills: %d"), Count);
		PlayerHUD->PlayerOverlay->KillCountText->SetText(FText::FromString(KillText));
	}
}

void ADefaultPlayerController::AddKill()
{
	SetHUDKillCount(++KillCount);
}

//
// Updating HUD for the XP Bar / Level.
//
void ADefaultPlayerController::SetHUDXP(float XP, float XPToNextLevel, int32 Level)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
			PlayerHUD->PlayerOverlay->XPBar &&
				PlayerHUD->PlayerOverlay->LevelText;

	if (bHUDValid)
	{
		const float XPPercent = XPToNextLevel > 0.f ? XP / XPToNextLevel : 0.f;
		PlayerHUD->PlayerOverlay->XPBar->SetPercent(XPPercent);

		FString LevelString = FString::Printf(TEXT("Level: %d"), Level);
		PlayerHUD->PlayerOverlay->LevelText->SetText(FText::FromString(LevelString));
	}
}
//
// 
//

void ADefaultPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
		SetHUDXP(PlayerCharacter->GetXP(), PlayerCharacter->GetXPToNextLevel(), PlayerCharacter->GetLevel());

		PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
		if (PlayerHUD)
		{
			PlayerHUD->InitHotbarForInventory(PlayerCharacter->GetInventory());
				PlayerHUD->InitInventoryScreenForInventory(PlayerCharacter->GetInventory());
		}
	}
}
