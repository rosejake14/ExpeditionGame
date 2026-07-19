// No Rights Reserved @ Team Expedition

#include "HUD/ShopSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UShopSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (BuyButton)
		BuyButton->OnClicked.AddDynamic(this, &UShopSlotWidget::HandleBuyButtonClicked);
}

void UShopSlotWidget::SetEntry(const FShopEntry& Entry)
{
	CachedId = Entry.Id;

	if (IconImage && Entry.Icon)
		IconImage->SetBrushFromTexture(Entry.Icon);
	if (NameText)
		NameText->SetText(Entry.Name);
	if (DescriptionText)
		DescriptionText->SetText(Entry.Description);
	if (StatusText)
		StatusText->SetText(Entry.StatusText);
	if (CostText)
		CostText->SetText(Entry.CostText);
	if (BuyButton)
		BuyButton->SetIsEnabled(Entry.bBuyEnabled);
}

void UShopSlotWidget::HandleBuyButtonClicked()
{
	OnBuyClicked.Broadcast(CachedId);
}
