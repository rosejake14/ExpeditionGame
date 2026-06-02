// No Rights Reserved @ Team Expedition 


#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

//void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
//{
//	RemoveFromParent();
//	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
//}

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn, bool ShowPlayerName)
{
	if (InPawn)
	{
		ENetRole LocalRole = InPawn->GetLocalRole();
		FString Role;
		FString PlayerName = GetPlayerName(InPawn->GetPlayerState());
		switch (LocalRole)
		{
			case(ROLE_Authority):
				Role = FString("Authority");
				break;
			case(ROLE_AutonomousProxy):
				Role = FString("AutonomousProxy");
				break;
			case(ROLE_SimulatedProxy):
				Role = FString("SimulatedProxy");
				break;
			case(ROLE_None):
				Role = FString("None");
				break;
		default:
			Role = FString("Error");
			break;
		}
		if (ShowPlayerName)
		{
			SetDisplayText(FString::Printf(TEXT("Local Role: %s"), *Role) + "\n" + FString::Printf(TEXT("Name: %s"), *PlayerName));
		}
		else
		{
			SetDisplayText(FString::Printf(TEXT("Local Role: %s"), *Role));
		}
		
	}
}

void UOverheadWidget::ShowPlayerRemoteRole(APawn* InPawn, bool ShowPlayerName)
{
	if (InPawn)
	{
		ENetRole RemoteRole = InPawn->GetRemoteRole();
		FString Role;
		FString PlayerName = GetPlayerName(InPawn->GetPlayerState());
		switch (RemoteRole)
		{
		case(ROLE_Authority):
			Role = FString("Authority");
			break;
		case(ROLE_AutonomousProxy):
			Role = FString("AutonomousProxy");
			break;
		case(ROLE_SimulatedProxy):
			Role = FString("SimulatedProxy");
			break;
		case(ROLE_None):
			Role = FString("None");
			break;
		default:
			Role = FString("Error");
			break;
		}
		if (ShowPlayerName)
		{
			SetDisplayText(FString::Printf(TEXT("Remote Role: %s"), *Role) + "\n" + FString::Printf(TEXT("Name: %s"), *PlayerName));
		}
		else
		{
			SetDisplayText(FString::Printf(TEXT("Remote Role: %s"), *Role));
		}
	}
}

FString UOverheadWidget::GetPlayerName(APlayerState* InPlayerState)
{
	if (InPlayerState)
	{
		FString PlayerName = InPlayerState->GetPlayerName();
		//SetDisplayText(DisplayText->GetText().ToString() + "\n" + PlayerName);
		return PlayerName;
	}
	return " ";
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
