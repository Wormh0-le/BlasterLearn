// Fill out your copyright notice in the Description page of Project Settings.


#include "BannerWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UBannerWidget::UpdateBanner(const FKillEventMessage& KillEventMessage)
{
	if (BannerStyleDataTable == nullptr)	return;
	const FName RowName = FName(*GetKillTypeDisplayName(KillEventMessage.KillType).ToString());
	BannerStyle = BannerStyleDataTable->FindRow<FKillBannerStyleRow>(RowName, TEXT("UpdateBanner"));
	if (BannerStyle)
	{
		if (BannerStyle->bConsecutiveKillState)
		{
			VerbIcon->SetText(FText::FromString("has been"));
			ObjectText->SetText(FText::FromName(RowName));
		}
		else
		{
			VerbIcon->SetText(FText::FromName(RowName));
			ObjectText->SetText(FText::FromString(KillEventMessage.VictimName));
		}
		SubjectText->SetText(FText::FromString(KillEventMessage.KillerName));
		const int32 PendingKillEventCount = KillEventCount.GetValue();
		Duration = FMath::Clamp(BannerStyle->MaxDuration / PendingKillEventCount, BannerStyle->MinDuration, BannerStyle->MaxDuration);
		KillEventCount.Decrement();
		if (BannerStyle->BackgroundMaterial)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BannerStyle->BackgroundMaterial, this);
			Background->SetBrushFromMaterial(DynMat);
		}
		Background->SetBrushColor(BannerStyle->BackgroundTint);
		if (BannerStyle->VoiceCue)
		{
			UGameplayStatics::PlaySound2D(this, BannerStyle->VoiceCue);
		}
	}
}
