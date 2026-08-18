// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UENUM()
enum class EEnemyAIState : uint8 { Roaming, Chasing };

UCLASS()
// TECH_DEBT(TD-ARCH-16): the FSM has only Roaming and Chasing — attacking is a timer bolted onto
// the chase refresh rather than a state, so there is no wind-up, no strafing, no ranged behaviour
// and no way for an archetype to override how it engages. Blocks enemy variety and bosses.
class EXPPRODEV_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAIPerceptionComponent* AIPerception;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	EEnemyAIState CurrentState = EEnemyAIState::Roaming;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	FVector HomeLocation;

	FTimerHandle RoamTimerHandle;
	FTimerHandle ChaseRefreshHandle;
	FTimerHandle AttackCooldownHandle;
	bool bCanAttack = true;

public:
	AEnemyAIController();

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float ChaseRefreshInterval = 0.3f;

	AActor* GetCurrentTarget() const { return TargetActor; }
	FVector GetHomeLocation()  const { return HomeLocation; }

	void ForceChase(AActor* Target);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnPerceptionForgotten(AActor* Actor);

	void PickRoamPoint();
	void StartChasing(AActor* Target);
	void StopChasing();
	void RefreshChase();
	void PerformAttack();
	void ResetAttackCooldown();
};
