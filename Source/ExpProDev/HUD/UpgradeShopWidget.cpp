// No Rights Reserved @ Team Expedition

#include "HUD/UpgradeShopWidget.h"
#include "HUD/UpgradeSlotWidget.h"
#include "Upgrade/UpgradeDefinition.h"
#include "Upgrade/UpgradeRegistry.h"
#include "Save/ExpProSaveGame.h"
#include "Save/SaveGameSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UUpgradeShopWidget::InitShop()
{
	// Load or create the save — shop works directly with the save file (no PlayerCharacter at main menu)
	CachedSave = USaveGameSubsystem::LoadActiveSlot(this);
	if (!CachedSave)
		CachedSave = Cast<UExpProSaveGame>(UGameplayStatics::CreateSaveGameObject(UExpProSaveGame::StaticClass()));

	if (!CachedSave || !Registry || !SlotWidgetClass) return;

	if (UpgradeListContainer)
	{
		UpgradeListContainer->ClearChildren();
		SlotWidgets.Empty();

		for (UUpgradeDefinition* Def : Registry->Upgrades)
		{
			if (!Def) continue;
			UUpgradeSlotWidget* SlotWidget = CreateWidget<UUpgradeSlotWidget>(GetOwningPlayer(), SlotWidgetClass);
			if (!SlotWidget) continue;

			const int32 Count = CachedSave->PurchasedUpgrades.FindRef(Def->UpgradeId);
			SlotWidget->SetUpgrade(Def, Count, CachedSave->DOSCoins);
			SlotWidget->OnBuyClicked.AddDynamic(this, &UUpgradeShopWidget::HandleUpgradePurchase);
			UpgradeListContainer->AddChild(SlotWidget);
			SlotWidgets.Add(SlotWidget);
		}
	}

	RefreshShop();
}

void UUpgradeShopWidget::HandleUpgradePurchase(UUpgradeDefinition* Def)
{
	if (!Def || !CachedSave) return;

	int32& CountRef = CachedSave->PurchasedUpgrades.FindOrAdd(Def->UpgradeId);
	const int32 Cost = Def->GetCostForNextPurchase(CountRef);

	if (Def->IsMaxed(CountRef) || CachedSave->DOSCoins < Cost) return;

	// Persist through the subsystem so PurchasedWeapons / other fields survive the write.
	const FName UpgradeId = Def->UpgradeId;
	USaveGameSubsystem::MutateActiveSlot(this, [UpgradeId, Cost](UExpProSaveGame& Save)
	{
		Save.DOSCoins -= Cost;
		Save.PurchasedUpgrades.FindOrAdd(UpgradeId)++;
	});

	// Keep the cached view in sync with what was just persisted.
	CachedSave->DOSCoins -= Cost;
	CountRef++;

	RefreshShop();
}

void UUpgradeShopWidget::RefreshShop()
{
	if (!CachedSave || !Registry) return;

	if (BalanceText)
		BalanceText->SetText(FText::Format(
			FText::FromString(TEXT("{0} DOS$")), FText::AsNumber(CachedSave->DOSCoins)));

	for (int32 i = 0; i < SlotWidgets.Num() && i < Registry->Upgrades.Num(); i++)
	{
		UUpgradeDefinition* Def = Registry->Upgrades[i];
		if (!SlotWidgets[i] || !Def) continue;
		const int32 Count = CachedSave->PurchasedUpgrades.FindRef(Def->UpgradeId);
		SlotWidgets[i]->SetUpgrade(Def, Count, CachedSave->DOSCoins);
	}
}
