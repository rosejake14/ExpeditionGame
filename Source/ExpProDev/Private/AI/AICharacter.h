// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AICharacter.generated.h"

// TECH_DEBT(TD-DEAD-2): DEAD CODE. Zero references anywhere in C++ or content — an empty ACharacter
// subclass left over from an early AI experiment. Delete along with AAICharacterController.
UCLASS()
class AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerceptionComp;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
