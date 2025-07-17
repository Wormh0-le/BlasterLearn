// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API UChatOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	void AddMessage(const FString& MessageTime, const FString& MessageRole, const FString& MessageInfo);
	
protected:
	UPROPERTY(meta=(BindWidget))
	class UScrollBox* SB_ChatHistory;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* ChatHistoryBackground;
	
	UPROPERTY(meta=(BindWidget))
	class UEditableTextBox* ChatInput;

private:
	UPROPERTY(EditAnywhere, Category = "ScrollItemSetting")
	TSubclassOf<UUserWidget> MessageItemWidgetClass;
};
