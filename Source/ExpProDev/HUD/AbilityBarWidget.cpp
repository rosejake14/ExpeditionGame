// No Rights Reserved @ Team Expedition

#include "HUD/AbilityBarWidget.h"

#include "HUD/AbilitySlotWidget.h"

void UAbilityBarWidget::InitAbilityBar(AHeroCharacter* InHero)
{
	// Each slot draws whichever ability its own Slot property names, so the bar itself only has
	// to hand the pawn down.
	if (PowerSlot)     PowerSlot->InitSlot(InHero);
	if (AddonSlot)     AddonSlot->InitSlot(InHero);
	if (UltimateSlot)  UltimateSlot->InitSlot(InHero);
}
