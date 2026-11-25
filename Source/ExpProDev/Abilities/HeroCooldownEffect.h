// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HeroCooldownEffect.generated.h"

/**
 *  The one cooldown effect every hero ability shares.
 *
 *  Its duration is SetByCaller rather than authored, so a single asset-free effect covers every
 *  ability's cooldown — UHeroAbility::ApplyCooldown fills in the seconds and tags the spec with
 *  the slot's cooldown tag. Without that, each ability would need its own cooldown asset.
 */
UCLASS()
class EXPPRODEV_API UHeroCooldownEffect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UHeroCooldownEffect();
};
