// No Rights Reserved @ Team Expedition

#include "HUD/WeaponSlotWidget.h"
#include "Weapon/WeaponDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UWeaponSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (BuyButton)
		BuyButton->OnClicked.AddDynamic(this, &UWeaponSlotWidget::HandleBuyButtonClicked);
}

void UWeaponSlotWidget::SetWeapon(UWeaponDefinition* Def, int32 OwnedCount, int32 PlayerCoins)
{
	CachedDef = Def;
	if (!Def) return;

	if (WeaponIcon && Def->Icon)
		WeaponIcon->SetBrushFromTexture(Def->Icon);
	if (WeaponNameText)
		WeaponNameText->SetText(Def->DisplayName);
	if (DescriptionText)
		DescriptionText->SetText(Def->Description);

	if (OwnedText)
		OwnedText->SetText(FText::Format(
			FText::FromString(TEXT("Owned: {0}")), FText::AsNumber(OwnedCount)));

	if (CostText)
		CostText->SetText(FText::Format(
			FText::FromString(TEXT("{0} DOS$")), FText::AsNumber(Def->Cost)));

	if (BuyButton)
		BuyButton->SetIsEnabled(PlayerCoins >= Def->Cost);
}

void UWeaponSlotWidget::HandleBuyButtonClicked()
{
	OnBuyClicked.Broadcast(CachedDef);
}
