// No Rights Reserved @ Team Expedition

#include "HUD/WeaponShopWidget.h"
#include "HUD/WeaponSlotWidget.h"
#include "Weapon/WeaponDefinition.h"
#include "Weapon/WeaponRegistry.h"
#include "Save/ExpProSaveGame.h"
#include "Save/SaveGameSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UWeaponShopWidget::InitShop()
{
	// Load the active-slot save, or create a fresh one if the slot is empty
	CachedSave = USaveGameSubsystem::LoadActiveSlot(this);
	if (!CachedSave)
		CachedSave = Cast<UExpProSaveGame>(UGameplayStatics::CreateSaveGameObject(UExpProSaveGame::StaticClass()));

	if (!CachedSave || !Registry || !SlotWidgetClass) return;

	if (WeaponListContainer)
	{
		WeaponListContainer->ClearChildren();
		SlotWidgets.Empty();

		for (UWeaponDefinition* Def : Registry->Weapons)
		{
			if (!Def) continue;
			UWeaponSlotWidget* SlotWidget = CreateWidget<UWeaponSlotWidget>(GetOwningPlayer(), SlotWidgetClass);
			if (!SlotWidget) continue;

			const int32 Owned = CachedSave->PurchasedWeapons.FindRef(Def->WeaponId);
			SlotWidget->SetWeapon(Def, Owned, CachedSave->DOSCoins);
			SlotWidget->OnBuyClicked.AddDynamic(this, &UWeaponShopWidget::HandleWeaponPurchase);
			WeaponListContainer->AddChild(SlotWidget);
			SlotWidgets.Add(SlotWidget);
		}
	}

	RefreshShop();
}

void UWeaponShopWidget::HandleWeaponPurchase(UWeaponDefinition* Def)
{
	if (!Def || !CachedSave) return;
	if (CachedSave->DOSCoins < Def->Cost) return;

	CachedSave->DOSCoins -= Def->Cost;
	CachedSave->PurchasedWeapons.FindOrAdd(Def->WeaponId)++;

	USaveGameSubsystem::SaveToActiveSlot(this, CachedSave);
	RefreshShop();
}

void UWeaponShopWidget::RefreshShop()
{
	if (!CachedSave || !Registry) return;

	if (BalanceText)
		BalanceText->SetText(FText::Format(
			FText::FromString(TEXT("{0} DOS$")), FText::AsNumber(CachedSave->DOSCoins)));

	for (int32 i = 0; i < SlotWidgets.Num() && i < Registry->Weapons.Num(); i++)
	{
		UWeaponDefinition* Def = Registry->Weapons[i];
		if (!SlotWidgets[i] || !Def) continue;
		const int32 Owned = CachedSave->PurchasedWeapons.FindRef(Def->WeaponId);
		SlotWidgets[i]->SetWeapon(Def, Owned, CachedSave->DOSCoins);
	}
}
