// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HeroRegistry.generated.h"

/**
 *  Single asset listing every hero in the game. ADefaultGameMode resolves the selected hero
 *  through this, and the hero select screen fills itself from it — so adding a hero is one new
 *  UHeroDefinition plus one entry here.
 *
 *  Mirrors UUpgradeRegistry and UWeaponRegistry.
 */
UCLASS(BlueprintType)
class EXPPRODEV_API UHeroRegistry : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	TArray<class UHeroDefinition*> Heroes;

	/** Find a definition by its save key. Null if nothing matches. */
	class UHeroDefinition* FindById(FName HeroId) const;

	/**
	 *  First valid entry, used as the fallback when no hero has been chosen — which is the normal
	 *  case when PIE starts directly on an island map instead of coming through the menu.
	 */
	class UHeroDefinition* GetDefaultHero() const;
};
