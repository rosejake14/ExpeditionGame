// No Rights Reserved @ Team Expedition

#include "HUD/UpgradeSlotWidget.h"
#include "Upgrade/UpgradeDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UUpgradeSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (BuyButton)
		BuyButton->OnClicked.AddDynamic(this, &UUpgradeSlotWidget::HandleBuyButtonClicked);
}

void UUpgradeSlotWidget::SetUpgrade(UUpgradeDefinition* Def, int32 CurrentLevel, int32 PlayerCoins)
{
	CachedDef = Def;
	if (!Def) return;

	if (UpgradeIcon && Def->Icon)
		UpgradeIcon->SetBrushFromTexture(Def->Icon);
	if (UpgradeNameText)
		UpgradeNameText->SetText(Def->DisplayName);
	if (DescriptionText)
		DescriptionText->SetText(Def->Description);

	const bool bMaxed = Def->IsMaxed(CurrentLevel);
	const int32 Cost  = Def->GetCostForNextPurchase(CurrentLevel);

	if (LevelText)
	{
		if (Def->MaxPurchases > 0)
			LevelText->SetText(FText::Format(
				FText::FromString(TEXT("Level: {0} / {1}")),
				FText::AsNumber(CurrentLevel), FText::AsNumber(Def->MaxPurchases)));
		else
			LevelText->SetText(FText::Format(
				FText::FromString(TEXT("Level: {0}")), FText::AsNumber(CurrentLevel)));
	}

	if (CostText)
	{
		if (bMaxed)
			CostText->SetText(FText::FromString(TEXT("MAX")));
		else
			CostText->SetText(FText::Format(
				FText::FromString(TEXT("{0} DOS$")), FText::AsNumber(Cost)));
	}

	if (BuyButton)
		BuyButton->SetIsEnabled(!bMaxed && PlayerCoins >= Cost);
}

void UUpgradeSlotWidget::HandleBuyButtonClicked()
{
	OnBuyClicked.Broadcast(CachedDef);
}
