// No Rights Reserved @ Team Expedition

#include "Core/ExpProGameplayTags.h"

namespace ExpProTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Slot_Signature, "Ability.Slot.Signature",
		"The hero's weapon action. Cooldown stands in for reload time.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Slot_Addon, "Ability.Slot.Addon",
		"Secondary attack. Cooldown-gated, or uncapped when no cooldown effect is set.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Slot_Ultimate, "Ability.Slot.Ultimate",
		"Gated by UltimateCharge rather than a cooldown.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage, "Data.Damage",
		"SetByCaller magnitude for incoming damage on GE_Damage.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_UltimateCharge, "Data.UltimateCharge",
		"SetByCaller magnitude for ultimate charge granted on an enemy kill.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Heal, "Data.Heal",
		"SetByCaller magnitude for health restored.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_CooldownDuration, "Data.CooldownDuration",
		"SetByCaller magnitude for an ability cooldown's duration, in seconds.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Signature, "Cooldown.Signature",
		"Present while the signature ability is cooling down.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Addon, "Cooldown.Addon",
		"Present while the addon ability is cooling down.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Ultimate, "Cooldown.Ultimate",
		"Present while the ultimate ability is cooling down.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead",
		"Applied while the player is eliminated. Blocks ability activation.");
}
