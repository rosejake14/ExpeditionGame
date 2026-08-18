// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class APlayerCharacter;

UINTERFACE(MinimalAPI)
class UInteractable : public UInterface { GENERATED_BODY() };

// TECH_DEBT(TD-ARCH-3): Interact is a plain C++ virtual rather than a BlueprintNativeEvent, so a
// Blueprint-only actor can never be interactable. There is also no GetInteractionText(), which is
// why nothing in the game can show a generic "Press E to ..." prompt — AExtractionZone had to grow
// its own bespoke prompt widget instead.
//
// TECH_DEBT(TD-ARCH-2): this is only ONE of the two interaction systems. Items, quest givers and
// the extraction zone route through here (APlayerCharacter::PendingInteractable), while weapons use
// an entirely separate path: AWeapon's AreaSphere overlap -> SetOverlappingWeapon -> the replicated
// OverlappingWeapon property -> a dedicated Equip key -> ServerEquipButtonPressed. Two overlap
// registrations, two input actions, two prompt mechanisms, for one player-facing concept.
class EXPPRODEV_API IInteractable
{
	GENERATED_BODY()
public:
	virtual void Interact(APlayerCharacter* Player) {}
};
