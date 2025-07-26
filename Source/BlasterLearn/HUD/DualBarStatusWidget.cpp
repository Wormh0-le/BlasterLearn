// Fill out your copyright notice in the Description page of Project Settings.


#include "DualBarStatusWidget.h"
#include "Components/ProgressBar.h"


void UDualBarStatusWidget::SetRedProgress(float percent)
{
	if (RedProgressBar)
	{
		RedProgressBar->SetPercent(percent);
	}
}

void UDualBarStatusWidget::SetBlueProgress(float percent)
{
	if (BlueProgressBar)
	{
		BlueProgressBar->SetPercent(percent);
	}
}

float UDualBarStatusWidget::GetBlueProgress()
{
	if (BlueProgressBar)
	{
		return BlueProgressBar->GetPercent(); 
	}
	return 0;
}

float UDualBarStatusWidget::GetRedProgress()
{
	if (RedProgressBar)
	{
		return RedProgressBar->GetPercent(); 
	}
	return 0;
}
