// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Inventory/LootTable.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class EXPPRODEV_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);

public:
	// ── Combat ────────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float Health = 100.f;

	// ── Rewards ───────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Rewards")
	float XPReward = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Rewards")
	FLootTable LootTable;

	// ── AI Behaviour (read by EnemyAIController on possess) ───────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	float SightRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	float LoseSightRadius = 2000.f;

	// Half-angle of the sight cone in degrees (60 = 120° FOV, 180 = all-around)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	float PeripheralVisionAngle = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	float RoamRadius = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	float RoamWaitMin = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	float RoamWaitMax = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float AttackRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	float AttackCooldown = 1.5f;

private:
	void DropLoot();
};
