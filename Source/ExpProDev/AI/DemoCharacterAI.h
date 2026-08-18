// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Inventory/LootTable.h"
#include "DemoCharacterAI.generated.h"

// TECH_DEBT(TD-DEAD-4): DEAD CODE. Superseded by AEnemyCharacter + AEnemyAIController. The only
// remaining C++ reference is a no-op cast in AProjectile::OnHit. BP_PlayerCharacter is reported to
// reference this class — clear that in-editor first, then delete this and DemoCharacterAIController.
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FLootTable LootTable;

	// XP awarded to whoever lands the killing blow.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	float XPReward = 25.f;
};
