// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatMessageItem.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API UChatMessageItem : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void Setup(const FString& MessageTime, const FString& MessageRole, const FString& MessageInfo);
private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TimeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;
};
