// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "HeroAbility.generated.h"

/** Which of a hero's three slots an ability fills. Drives input, HUD position and cooldown tag. */
UENUM(BlueprintType)
enum class EHeroAbilitySlot : uint8
{
	Signature UMETA(DisplayName = "Signature"),
	Addon     UMETA(DisplayName = "Addon"),
	Ultimate  UMETA(DisplayName = "Ultimate")
};

/**
 *  Base class for every hero ability.
 *
 *  Two things it adds on top of UGameplayAbility:
 *
 *   1. **A data-driven cooldown.** Set CooldownSeconds and the shared UHeroCooldownEffect is
 *      applied with that duration, tagged for this ability's slot. No per-ability cooldown asset.
 *      Zero seconds means no cooldown at all — that is how "unlimited use" abilities are authored.
 *
 *   2. **Charge gating for ultimates.** Set bChargeGated and the ability refuses to activate
 *      until UltimateCharge has reached MaxUltimateCharge, then spends the whole meter. Ultimates
 *      build from kills rather than ticking down, so they do not use the cooldown path at all.
 *
 *  Presentation is left to Blueprint via OnAbilityActivated — montages, VFX and sound belong
 *  there, not here.
 */
UCLASS(Abstract, Blueprintable)
class EXPPRODEV_API UHeroAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UHeroAbility();

	// ── Presentation ──────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability")
	class UTexture2D* Icon = nullptr;

	// ── Behaviour ─────────────────────────────────────────────────────────

	/** Which slot this fills. Set in each ability's constructor; drives the cooldown tag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability")
	EHeroAbilitySlot Slot = EHeroAbilitySlot::Signature;

	/** Seconds before it can be used again. 0 = no cooldown. Ignored when bChargeGated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability", meta = (ClampMin = 0.0))
	float CooldownSeconds = 0.f;

	/** Ultimates: needs a full charge meter, and spends it on activation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Ability")
	bool bChargeGated = false;

	/** Blueprint presentation hook — montage, VFX, sound. Fired after the ability commits. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Ability")
	void OnAbilityActivated();

	/** The cooldown tag for a given slot. Used by the HUD as well as the cooldown itself. */
	static FGameplayTag GetCooldownTagForSlot(EHeroAbilitySlot InSlot);

	// ── UGameplayAbility ──────────────────────────────────────────────────

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	/**
	 *  Where a subclass puts its actual effect. Called after the ability has committed, so the
	 *  cooldown is already running and any charge already spent — just do the thing.
	 */
	virtual void ExecuteHeroAbility(const FGameplayAbilityActorInfo* ActorInfo) {}

	/** The hero that activated this. Null if the ability is somehow running without one. */
	class AHeroCharacter* GetHeroCharacter(const FGameplayAbilityActorInfo* ActorInfo) const;

private:
	/**
	 *  Rebuilt on demand rather than in the constructor, because Slot can be changed on a
	 *  Blueprint subclass after construction. Mutable so the const GetCooldownTags can fill it
	 *  and hand back a stable pointer, which is what GAS expects.
	 */
	mutable FGameplayTagContainer CooldownTags;
};
