// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Roam   UMETA(DisplayName = "Roam"),
	Attack UMETA(DisplayName = "Attack"),
};

UCLASS()
class EXPPRODEV_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	FORCEINLINE EEnemyState GetCurrentState() const { return CurrentState; }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UAIPerceptionComponent* AIPerception;

	UPROPERTY()
	class UAISenseConfig_Sight* SightConfig;

	EEnemyState CurrentState = EEnemyState::Roam;

	UPROPERTY()
	TWeakObjectPtr<AActor> AttackTarget;

	FVector HomeLocation;

	FTimerHandle RoamWaitHandle;
	FTimerHandle AttackChaseHandle;

	void SetState(EEnemyState NewState);

	void PickNewRoamTarget();
	void BeginChase();

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
