// No Rights Reserved @ Team Expedition 

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IWebSocket.h"
#include "RemotePlayer.h"
#include "WebSocketGameInstance.generated.h"

// TECH_DEBT(TD-DEAD-1): DEAD PROTOTYPE. This is NOT the game's GameInstance — no GameInstanceClass
// is set in Config/DefaultEngine.ini, so the engine uses the stock UGameInstance and none of this
// ever runs. The WebSockets/ folder (this class, UWebSocketClient, ARemotePlayer,
// AWSPlayerCharacter) is the only consumer of the WebSockets, Json and JsonUtilities module
// dependencies, and WebSocketGameInstance.cpp includes Variant_Shooter/ShooterCharacter.h, which
// keeps that dead variant linked too. Delete this folder first — it unblocks TD-DEAD-8.
UCLASS()
class EXPPRODEV_API UWebSocketGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable)
	void SendMovement(const FVector& Position, const FRotator& Rotation);

	UFUNCTION(BlueprintCallable)
	void SendHitcast(const FHitResult& HitResult);

	TSharedPtr<IWebSocket> Socket;

private:
	FString CurrentPlayerId;
	TMap<FString, ARemotePlayer*> RemotePlayers;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARemotePlayer> RemotePlayerClass;

	UPROPERTY(EditAnywhere)
	FVector TargetPosition;
	UPROPERTY(EditAnywhere)
	FRotator TargetRotation;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float LerpAlpha = 15.0f; 
};
