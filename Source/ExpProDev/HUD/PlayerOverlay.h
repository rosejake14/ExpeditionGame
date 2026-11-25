// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerOverlay.generated.h"

class ADefaultGameMode;

/**
 *  Always-on HUD overlay: health, XP / level, kill count and the run timer.
 *
 *  Everything but the timer is pushed in from ADefaultPlayerController's SetHUD* functions. The
 *  timer instead polls ADefaultGameMode on a repeating timer, because pushing a value that changes
 *  every second through the controller would mean yet another near-identical push function.
 */
UCLASS()
class EXPPRODEV_API UPlayerOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KillCountText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* XPBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LevelText;

	/**
	 *  Run clock, drawn as "M:SS". Optional rather than a hard BindWidget so existing
	 *  WBP_PlayerOverlay assets keep compiling until the TextBlock has been added to them.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* RunTimerText;

	/** Colour of the clock while it counts down towards zero. */
	UPROPERTY(EditAnywhere, Category = "Run Timer")
	FLinearColor RunTimerNormalColour = FLinearColor::White;

	/** Colour of the clock once it has passed zero and is counting upwards. */
	UPROPERTY(EditAnywhere, Category = "Run Timer")
	FLinearColor RunTimerOvertimeColour = FLinearColor::Red;

	/** How often the clock is re-read, in seconds. Well under a second so the display never skips. */
	UPROPERTY(EditAnywhere, Category = "Run Timer", meta = (ClampMin = "0.01"))
	float RunTimerRefreshRate = 0.1f;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	/** Re-reads the game mode's clock and repaints RunTimerText if the displayed second changed. */
	void UpdateRunTimer();

	/** Cached so we aren't casting the game mode ten times a second. */
	TWeakObjectPtr<ADefaultGameMode> RunTimerGameMode;

	FTimerHandle RunTimerRefreshHandle;

	/** Last whole-second value written out; -1 forces the first update through. */
	int32 LastDisplayedSeconds = -1;

	/** Which colour is currently applied, so SetColorAndOpacity only runs on the transition. */
	bool bOvertimeColourApplied = false;
};
