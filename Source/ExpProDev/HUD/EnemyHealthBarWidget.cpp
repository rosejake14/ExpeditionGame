// No Rights Reserved @ Team Expedition

#include "HUD/EnemyHealthBarWidget.h"
#include "Components/ProgressBar.h"

void UEnemyHealthBarWidget::SetHealthPercent(float Percent)
{
	if (HealthBar)
		HealthBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
}
