// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "ChatWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void AddMessage(const FString& MessageTime, const FString& MessageRole, const FString& MessageInfo);

	void EnableChatInput();

	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	virtual FReply NativeOnPreviewKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta=(BindWidget))
	UEditableTextBox* ChatInput;
	
protected:
	UPROPERTY(meta=(BindWidget))
	class UScrollBox* SB_ChatHistory;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* ChatHistoryBackground;

private:
	UPROPERTY(EditAnywhere, Category = "ScrollItemSetting")
	TSubclassOf<UUserWidget> MessageItemWidgetClass;

	bool bIsChatting = false;
};
