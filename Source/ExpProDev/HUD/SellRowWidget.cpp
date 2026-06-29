// No Rights Reserved @ Team Expedition

#include "HUD/SellRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void USellRowWidget::SetEntry(const FSellEntry& Entry)
{
	if (ItemIconImage && Entry.Icon)
		ItemIconImage->SetBrushFromTexture(Entry.Icon);
	if (QuantityText)
		QuantityText->SetText(FText::AsNumber(Entry.Quantity));
	if (CoinsEarnedText)
		CoinsEarnedText->SetText(FText::Format(
			FText::FromString(TEXT("{0} DOS$")), FText::AsNumber(Entry.CoinsEarned)));
}
