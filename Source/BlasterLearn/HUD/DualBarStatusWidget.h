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
	class UTextBlock* TeleportText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* BlueProgress;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* RedProgress;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* BlueActivating;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* RedActivating;
};
