// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityBarWidget.generated.h"

/**
 *  The three-slot ability bar. Set the parent class of WBP_AbilityBar to this.
 *
 *  The Blueprint must contain three UAbilitySlotWidget children with these exact names:
 *    PowerSlot / AddonSlot / UltimateSlot
 *  Set each one's Slot property to match its name.
 *
 *  Follows the same shape as UHotbarWidget: the controller hands it the pawn once both exist.
 */
UCLASS()
class EXPPRODEV_API UAbilityBarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** Called from ADefaultPlayerController::OnPossess once the HUD and pawn both exist. */
	void InitAbilityBar(class AHeroCharacter* InHero);

protected:
	UPROPERTY(meta = (BindWidget))
	class UAbilitySlotWidget* PowerSlot;

	UPROPERTY(meta = (BindWidget))
	class UAbilitySlotWidget* AddonSlot;

	UPROPERTY(meta = (BindWidget))
	class UAbilitySlotWidget* UltimateSlot;
};
