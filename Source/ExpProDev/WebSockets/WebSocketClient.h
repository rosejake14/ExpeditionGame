// Source/ExpProDev/WebSocketClient.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IWebSocket.h"
#include "WebSocketClient.generated.h"

UCLASS(Blueprintable, BlueprintType)
class EXPPRODEV_API UWebSocketClient : public UObject
{
	GENERATED_BODY()

public:
	UWebSocketClient();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void Connect(const FString& Url);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendMovement(float X, float Y, float Z);
	void SendRotation(float Yaw);
	void SendShoot();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void Close();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket")
	TSubclassOf<AActor> RemotePlayerClass;

	UPROPERTY(BlueprintReadOnly, Category = "WebSocket")
	int32 MyPlayerId = -1;

	UPROPERTY()
	TMap<int32, AActor*> RemotePlayers;  // PlayerId -> Remote Actor

private:
	TSharedPtr<IWebSocket> Socket;

	void OnConnected();
	void OnConnectionError(const FString& Error);
	void OnMessage(const FString& Message);
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
};