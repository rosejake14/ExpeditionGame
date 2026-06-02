// Source/ExpProDev/WebSocketClient.cpp
#include "WebSocketClient.h"
#include "WebSocketsModule.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Engine/World.h"
#include "RemotePlayer.h"
#include "GameFramework/PlayerController.h"

UWebSocketClient::UWebSocketClient()
{
}

void UWebSocketClient::Connect(const FString& Url)
{
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
    }

    Socket = FWebSocketsModule::Get().CreateWebSocket(Url);

    Socket->OnConnected().AddUObject(this, &UWebSocketClient::OnConnected);
    Socket->OnConnectionError().AddUObject(this, &UWebSocketClient::OnConnectionError);
    Socket->OnMessage().AddUObject(this, &UWebSocketClient::OnMessage);
    Socket->OnClosed().AddUObject(this, &UWebSocketClient::OnClosed);

    Socket->Connect();
}

void UWebSocketClient::SendMessage(const FString& Message)
{
    if (Socket.IsValid() && Socket->IsConnected())
    {
        Socket->Send(Message);
    }
}

void UWebSocketClient::SendMovement(float X, float Y, float Z)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("action"), TEXT("move"));
    JsonObject->SetNumberField(TEXT("x"), X);
    JsonObject->SetNumberField(TEXT("y"), Y);
    JsonObject->SetNumberField(TEXT("z"), Z);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    SendMessage(OutputString);
}

void UWebSocketClient::SendRotation(float Yaw)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("type"), TEXT("rot"));
    JsonObject->SetNumberField(TEXT("yaw"), Yaw);

    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    SendMessage(Output);
}

void UWebSocketClient::SendShoot()
{
    TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
    Json->SetStringField("type", "shoot");

    FString Out;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
    FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);
    SendMessage(Out);
}

void UWebSocketClient::Close()
{
    if (Socket.IsValid())
    {
        Socket->Close();
    }
}

void UWebSocketClient::OnConnected()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket Connected!"));
}

void UWebSocketClient::OnConnectionError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("WebSocket Connection Error: %s"), *Error);
}

void UWebSocketClient::OnMessage(const FString& Message)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON: %s"), *Message);
        return;
    }

    FString Type = JsonObject->GetStringField(TEXT("type"));

    if (Type == TEXT("welcome"))
    {
        MyPlayerId = JsonObject->GetIntegerField(TEXT("playerId"));
        UE_LOG(LogTemp, Log, TEXT("My Player ID: %d"), MyPlayerId);
    }
    else if (Type == TEXT("update"))
    {
        int32 PlayerId = JsonObject->GetIntegerField(TEXT("playerId"));
        float X = JsonObject->GetNumberField(TEXT("x"));
        float Y = JsonObject->GetNumberField(TEXT("y"));
        float Z = JsonObject->GetNumberField(TEXT("z"));
        FVector NewPos(X, Y, Z);

        if (PlayerId == MyPlayerId) return;  // Ignore self

        AActor** FoundActor = RemotePlayers.Find(PlayerId);
        if (FoundActor && *FoundActor && (*FoundActor)->IsValidLowLevel())
        {
            if (ARemotePlayer* Remote = Cast<ARemotePlayer>(*FoundActor))
            {
                Remote->SetRemoteLocation(NewPos);
            }
        }
        else if (RemotePlayerClass && GetWorld())
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            AActor* NewActor = GetWorld()->SpawnActor<AActor>(RemotePlayerClass, NewPos, FRotator::ZeroRotator, SpawnParams);
            if (NewActor)
            {
                RemotePlayers.Add(PlayerId, NewActor);
                UE_LOG(LogTemp, Log, TEXT("Spawned remote player %d at %s"), PlayerId, *NewPos.ToString());
            }
        }
    }
}

void UWebSocketClient::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Warning, TEXT("WebSocket Closed (%d): %s (Clean=%d)"), StatusCode, *Reason, bWasClean);

    for (auto& Pair : RemotePlayers)
    {
        if (Pair.Value && Pair.Value->IsValidLowLevel())
        {
            Pair.Value->Destroy();
        }
    }
    RemotePlayers.Empty();
}