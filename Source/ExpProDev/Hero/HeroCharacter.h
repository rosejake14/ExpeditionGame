// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HeroAbility.h"
#include "Character/PlayerCharacter.h"
#include "GameplayAbilitySpec.h"
#include "HeroCharacter.generated.h"

class UHeroDefinition;
class UInputAction;

/**
 *  The pawn every hero uses. It holds no per-hero logic of its own — it reads a
 *  UHeroDefinition and applies it, so all four heroes are the same class with different data.
 *
 *  ADefaultGameMode spawns this deferred, calls SetHeroDefinition, then finishes the spawn, so
 *  appearance is already correct before BeginPlay and possession run.
 *
 *  Subclass this only for a hero that needs genuinely bespoke C++ — see doc/HeroSystem.md §4.
 */
UCLASS()
class EXPPRODEV_API AHeroCharacter : public APlayerCharacter
{
	GENERATED_BODY()
public:
	/**
	 *  Assigns the hero this pawn represents and immediately applies its appearance.
	 *  Call between SpawnActorDeferred and FinishSpawningActor.
	 */
	void SetHeroDefinition(UHeroDefinition* InDefinition);

	UFUNCTION(BlueprintPure, Category = "Hero")
	UHeroDefinition* GetHeroDefinition() const { return HeroDefinition; }

	/** The ability filling a slot, or null if this hero has none there. Used by the HUD. */
	UFUNCTION(BlueprintPure, Category = "Hero")
	UHeroAbility* GetAbilityForSlot(EHeroAbilitySlot InSlot) const;

	//
	// Ability input. These are the same three keys for every hero, so they live on the pawn
	// rather than being repeated in all four hero definitions.
	//

	UPROPERTY(EditAnywhere, Category = "Hero|Input")
	UInputAction* SignatureAbilityAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Hero|Input")
	UInputAction* AddonAbilityAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Hero|Input")
	UInputAction* UltimateAbilityAction = nullptr;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	/** Takes starting stats from the definition rather than the inherited BaseMaxHealth. */
	virtual void InitialiseAttributes() override;

	/** Mesh, animation, material, scale and capsule. Anything unset is left as the pawn's own. */
	void ApplyAppearance();

	/** Gives the three abilities the definition names, remembering each handle for input. */
	void GrantAbilities();

	/** Input handlers — one per slot, each activating that slot's granted spec. */
	void ActivateSignature();
	void ActivateAddon();
	void ActivateUltimate();

	/** Shared body of the three handlers above. */
	void ActivateSlot(EHeroAbilitySlot InSlot);

private:
	UPROPERTY()
	UHeroDefinition* HeroDefinition = nullptr;

	/**
	 *  Granted ability handles, indexed by EHeroAbilitySlot. Activating by handle rather than by
	 *  tag keeps input wiring independent of how an ability tags itself.
	 */
	FGameplayAbilitySpecHandle AbilityHandles[3];
};
