// No Rights Reserved @ Team Expedition

#include "HUD/ShopWidgetBase.h"
#include "Economy/EconomySubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UShopWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (UEconomySubsystem* Econ = GetEconomy())
	{
		Econ->OnBalanceChanged.AddDynamic(this, &UShopWidgetBase::HandleBalanceChanged);
		Econ->OnPurchaseCompleted.AddDynamic(this, &UShopWidgetBase::HandlePurchaseCompleted);
	}

	InitShop();
}

void UShopWidgetBase::NativeDestruct()
{
	if (UEconomySubsystem* Econ = GetEconomy())
	{
		Econ->OnBalanceChanged.RemoveDynamic(this, &UShopWidgetBase::HandleBalanceChanged);
		Econ->OnPurchaseCompleted.RemoveDynamic(this, &UShopWidgetBase::HandlePurchaseCompleted);
	}

	Super::NativeDestruct();
}

UEconomySubsystem* UShopWidgetBase::GetEconomy() const
{
	return UEconomySubsystem::Get(this);
}

void UShopWidgetBase::InitShop()
{
	if (!SlotWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: SlotWidgetClass is unset — set it in the widget BP class defaults."), *GetName());
		return;
	}

	const TArray<FShopEntry> Entries = BuildEntries();

	if (ListContainer)
	{
		ListContainer->ClearChildren();
		SlotWidgets.Empty();

		for (const FShopEntry& Entry : Entries)
		{
			UShopSlotWidget* SlotWidget = CreateWidget<UShopSlotWidget>(GetOwningPlayer(), SlotWidgetClass);
			if (!SlotWidget) continue;

			SlotWidget->SetEntry(Entry);
			SlotWidget->OnBuyClicked.AddDynamic(this, &UShopWidgetBase::HandleBuyClicked);
			ListContainer->AddChild(SlotWidget);
			SlotWidgets.Add(SlotWidget);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s: no ListContainer bound — name the shop's scroll box 'ListContainer'."), *GetName());
	}

	if (UEconomySubsystem* Econ = GetEconomy())
		SetBalanceText(Econ->GetBalance());
}

void UShopWidgetBase::RefreshShop()
{
	const TArray<FShopEntry> Entries = BuildEntries();

	// Row count is registry-driven and fixed; if it ever diverges, rebuild from scratch.
	if (Entries.Num() != SlotWidgets.Num())
	{
		InitShop();
		return;
	}

	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (SlotWidgets[i])
			SlotWidgets[i]->SetEntry(Entries[i]);
	}
}

void UShopWidgetBase::SetBalanceText(int32 Balance)
{
	if (BalanceText)
		BalanceText->SetText(FText::Format(
			FText::FromString(TEXT("{0} DOS$")), FText::AsNumber(Balance)));
}

void UShopWidgetBase::HandleBuyClicked(FName Id)
{
	// Delegate the transaction to the subsystem; the resulting OnBalanceChanged /
	// OnPurchaseCompleted broadcasts drive the refresh, so no state is synced by hand here.
	PurchaseEntry(Id);
}

void UShopWidgetBase::HandleBalanceChanged(int32 NewBalance)
{
	SetBalanceText(NewBalance);
	RefreshShop();
}

void UShopWidgetBase::HandlePurchaseCompleted(FName Id)
{
	RefreshShop();
}
