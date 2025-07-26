// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DualBarStatusWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API UDualBarStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StatusText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* BlueProgressBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* RedProgressBar;

	void SetRedProgress(float percent);

	void SetBlueProgress(float percent);

	float GetBlueProgress();

	float GetRedProgress();

	void SetStatusInfo(const FText& InfoText);
};
