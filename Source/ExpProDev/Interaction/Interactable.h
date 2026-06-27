// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class APlayerCharacter;

UINTERFACE(MinimalAPI)
class UInteractable : public UInterface { GENERATED_BODY() };

class EXPPRODEV_API IInteractable
{
	GENERATED_BODY()
public:
	virtual void Interact(APlayerCharacter* Player) {}
};
