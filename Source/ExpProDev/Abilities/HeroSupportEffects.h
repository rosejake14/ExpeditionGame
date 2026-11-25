// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HeroSupportEffects.generated.h"

/**
 *  Instant heal. SetByCaller magnitude on ExpProTags::Data_Heal, added straight to Health;
 *  UHeroAttributeSet clamps it to MaxHealth.
 */
UCLASS()
class EXPPRODEV_API UHealEffect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UHealEffect();
};

/**
 *  Instant grant of ultimate charge. SetByCaller magnitude on ExpProTags::Data_UltimateCharge.
 *  Applied on every enemy kill — see APlayerCharacter::GrantUltimateCharge.
 */
UCLASS()
class EXPPRODEV_API UUltimateChargeEffect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UUltimateChargeEffect();
};

/**
 *  Instant spend of the whole ultimate meter. A flat -1x MaxUltimateCharge would not work
 *  (the max varies per hero), so the amount is SetByCaller too.
 */
UCLASS()
class EXPPRODEV_API UUltimateSpendEffect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UUltimateSpendEffect();
};
