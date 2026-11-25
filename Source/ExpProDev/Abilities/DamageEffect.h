// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DamageEffect.generated.h"

/**
 *  The one route damage takes into a hero's attributes.
 *
 *  An instant effect carrying a single SetByCaller modifier on UHeroAttributeSet::Damage.
 *  Callers build a spec, set the ExpProTags::Data_Damage magnitude to the damage amount, and
 *  apply it; UHeroAttributeSet::PostGameplayEffectExecute converts that into a Health change.
 *
 *  Defined in C++ rather than as a Blueprint asset so the damage path stays reviewable in
 *  source and cannot be silently reconfigured in the editor.
 *
 *  Used by APlayerCharacter::RecieveDamage, which still receives damage from enemies through
 *  the stock OnTakeAnyDamage delegate — no enemy code needed changing.
 */
UCLASS()
class EXPPRODEV_API UDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UDamageEffect();
};
