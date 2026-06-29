// No Rights Reserved @ Team Expedition

#include "HUD/SellSummaryWidget.h"
#include "HUD/SellRowWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void USellSummaryWidget::InitSummary(const TArray<FSellEntry>& Entries, int32 TotalEarned, int32 NewBalance)
{
	if (ItemListContainer)
	{
		ItemListContainer->ClearChildren();
		for (const FSellEntry& Entry : Entries)
		{
			if (!RowWidgetClass) continue;
			USellRowWidget* Row = CreateWidget<USellRowWidget>(GetOwningPlayer(), RowWidgetClass);
			if (Row)
			{
				Row->SetEntry(Entry);
				ItemListContainer->AddChild(Row);
			}
		}
	}

	if (TotalCoinsText)
		TotalCoinsText->SetText(FText::Format(
			FText::FromString(TEXT("+{0} DOS$")), FText::AsNumber(TotalEarned)));

	if (TotalBalanceText)
		TotalBalanceText->SetText(FText::Format(
			FText::FromString(TEXT("Total Balance: {0} DOS$")), FText::AsNumber(NewBalance)));
}
