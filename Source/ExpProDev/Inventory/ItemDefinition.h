// No Rights Reserved @ Team Expedition

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
// UDataAsset is used instead of UPrimaryDataAsset for reliable editor asset creation flow
#include "ItemDefinition.generated.h"

// Item Type
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Loot		UMETA(DisplayName = "Loot"),
	Consumable	UMETA(DisplayName = "Consumable"),
	Skill		UMETA(DisplayName = "Skill"),
	Weapon		UMETA(DisplayName = "Weapon")
};

UCLASS(BlueprintType)
class EXPPRODEV_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	class UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemType ItemType = EItemType::Loot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 1))
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float BaseValue = 0.f;

	// If true, picked up immediately on overlap. If false, player must press Interact (E).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bAutoPickup = false;

	// Only used when ItemType == Weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<class AWeapon> WeaponClass;
};
