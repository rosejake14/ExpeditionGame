// No Rights Reserved @ Team Expedition

#include "HUD/ExtractPromptWidget.h"
#include "Components/TextBlock.h"

void UExtractPromptWidget::SetPromptText(const FText& Text)
{
	if (PromptText)
		PromptText->SetText(Text);
}
