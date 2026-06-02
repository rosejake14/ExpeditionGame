// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DemoCharacterAI.generated.h"

UCLASS()
class EXPPRODEV_API ADemoCharacterAI : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADemoCharacterAI();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION() // Callback events ALWAYS need to be UFUNCTIONS
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatedController, class AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerceptionComp;

	//UPROPERTY(EditAnywhere)
	//TSubclassOf<AController> AIController;
	//UPROPERTY()
	//class ADemoCharacterAIController* AIController;

	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float MaxHealth = 100.f;
	UPROPERTY(Replicated, VisibleAnywhere, Category = PlayerStats)
	float Health;
};
