// No Rights Reserved @ Team Expedition

#include "Core/ExpProGameInstance.h"

#include "ExpProDev.h"

#include "AbilitySystemGlobals.h"
#include "Kismet/GameplayStatics.h"

void UExpProGameInstance::Init()
{
	Super::Init();

	// GAS needs its globals built once per process before any ability runs. Skipping this does
	// not fail here — it crashes later, the first time an ability uses target data.
	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const bool bAlreadyInitialised = AbilitySystemGlobals.IsAbilitySystemGlobalsInitialized();
	if (!bAlreadyInitialised)
	{
		AbilitySystemGlobals.InitGlobalData();
	}

	// Logged unconditionally and outside the branch above, because this line does double duty: it
	// is also the only proof that GameInstanceClass actually resolved to this class. The editor
	// often builds the ability system globals during its own startup, so a log inside the branch
	// stays silent even when everything is wired correctly — which makes a missing line
	// impossible to tell apart from a misconfigured GameInstanceClass.
	UE_LOG(LogExpProDev, Log, TEXT("ExpProGameInstance: init complete (ability system globals %s)."),
		bAlreadyInitialised ? TEXT("were already built") : TEXT("built here"));
}

UExpProGameInstance* UExpProGameInstance::Get(const UObject* WorldContext)
{
	return Cast<UExpProGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
}

void UExpProGameInstance::SetSelectedHeroId(FName HeroId)
{
	SelectedHeroId = HeroId;
	UE_LOG(LogExpProDev, Log, TEXT("ExpProGameInstance: hero set to '%s'."), *HeroId.ToString());
}

void UExpProGameInstance::SetSelectedLevel(TSoftObjectPtr<UWorld> Level)
{
	SelectedLevel = Level;
	UE_LOG(LogExpProDev, Log, TEXT("ExpProGameInstance: level set to '%s'."), *Level.ToString());
}

bool UExpProGameInstance::OpenSelectedLevel()
{
	if (SelectedLevel.IsNull())
	{
		UE_LOG(LogExpProDev, Warning,
			TEXT("ExpProGameInstance: OpenSelectedLevel called with no level chosen — ignoring."));
		return false;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, SelectedLevel);
	return true;
}
