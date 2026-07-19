// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

// Simple floating health bar shown above an enemy. The widget BP must contain a UProgressBar
// named 'HealthBar'; the enemy drives it via SetHealthPercent on damage.
UCLASS()
class EXPPRODEV_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Fill amount, 0..1 (clamped).
	void SetHealthPercent(float Percent);

protected:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
};
