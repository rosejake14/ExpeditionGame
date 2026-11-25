// No Rights Reserved @ Team Expedition


#include "HUD/PlayerOverlay.h"

#include "Gamemode/DefaultGameMode.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UPlayerOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	// A repeating timer rather than NativeTick: UUserWidget only ticks when its generated class was
	// compiled knowing the native parent needs it, so a NativeTick override added in C++ silently
	// does nothing against an already-compiled WBP. A timer is immune to that, and ten reads a
	// second is ample for a display that only changes once a second.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RunTimerRefreshHandle, this,
			&UPlayerOverlay::UpdateRunTimer, RunTimerRefreshRate, true);
	}

	UpdateRunTimer();
}

void UPlayerOverlay::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RunTimerRefreshHandle);
	}

	Super::NativeDestruct();
}

void UPlayerOverlay::UpdateRunTimer()
{
	if (!RunTimerText) return;

	if (!RunTimerGameMode.IsValid())
	{
		const UWorld* World = GetWorld();
		RunTimerGameMode = World ? Cast<ADefaultGameMode>(World->GetAuthGameMode()) : nullptr;
	}

	const ADefaultGameMode* GameMode = RunTimerGameMode.Get();
	const bool bShowTimer = GameMode && GameMode->IsRunTimerRunning();

	// Hide rather than leave a stale "0:00" up on maps that never start a run.
	const ESlateVisibility DesiredVisibility = bShowTimer
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed;
	if (RunTimerText->GetVisibility() != DesiredVisibility)
	{
		RunTimerText->SetVisibility(DesiredVisibility);
	}

	if (!bShowTimer) return;

	const float Remaining = GameMode->GetRunTimeRemaining();
	const bool bOvertime = Remaining < 0.f;

	// Counting down we ceil, so the final second reads "0:01" and "0:00" lands exactly at zero.
	// Counting up we floor, so the first overtime second reads "0:00" before ticking to "0:01".
	// Getting this the other way round makes the display skip or repeat a second across zero.
	const int32 TotalSeconds = bOvertime
		? FMath::FloorToInt(-Remaining)
		: FMath::CeilToInt(Remaining);

	if (TotalSeconds != LastDisplayedSeconds)
	{
		LastDisplayedSeconds = TotalSeconds;
		RunTimerText->SetText(FText::FromString(
			FString::Printf(TEXT("%d:%02d"), TotalSeconds / 60, TotalSeconds % 60)));
	}

	if (bOvertime != bOvertimeColourApplied)
	{
		bOvertimeColourApplied = bOvertime;
		RunTimerText->SetColorAndOpacity(
			FSlateColor(bOvertime ? RunTimerOvertimeColour : RunTimerNormalColour));
	}
}
