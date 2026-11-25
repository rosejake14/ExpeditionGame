// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HeroAbility.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySlotWidget.generated.h"

/**
 *  One ability on the bar. Set the parent class of WBP_AbilitySlot to this.
 *
 *  It computes state every frame and hands it to Blueprint through the read-only values below,
 *  so the visual treatment stays in the Blueprint. Bind the icon, a radial or progress bar to
 *  FillFraction, and the ready state to bIsReady.
 *
 *  FillFraction means different things per slot on purpose: for a cooldown ability it is how
 *  much of the cooldown has elapsed (0 just used, 1 ready); for an ultimate it is how full the
 *  charge meter is. Both read naturally as "fills up until you can use it".
 */
UCLASS()
class EXPPRODEV_API UAbilitySlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 *  Which slot this widget draws. Set on the widget instance inside WBP_AbilityBar.
	 *  Named AbilitySlot rather than Slot because UWidget already has a member called Slot
	 *  (its panel slot), and UHT rejects shadowing it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	EHeroAbilitySlot AbilitySlot = EHeroAbilitySlot::Power;

	/** Called by UAbilityBarWidget once the pawn exists. */
	void InitSlot(class AHeroCharacter* InHero);

	// ── State for the Blueprint to draw ───────────────────────────────────

	/** 0..1. Cooldown progress for normal abilities, charge progress for ultimates. */
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	float FillFraction = 1.f;

	/** True when the ability can actually be activated right now. */
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	bool bIsReady = false;

	/** False when this hero has no ability in this slot — hide the widget. */
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	bool bHasAbility = false;

	/** Seconds left on the cooldown, for slots that want to print a number. 0 when ready. */
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	float CooldownRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	FText AbilityName;

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	class UTexture2D* AbilityIcon = nullptr;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY()
	class AHeroCharacter* Hero = nullptr;
};
