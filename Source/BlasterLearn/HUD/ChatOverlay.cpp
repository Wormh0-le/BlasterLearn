// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatOverlay.h"
#include "ChatMessageItem.h"
#include "BlasterLearn/PlayerController/BlasterPlayerController.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"


void UChatOverlay::AddMessage(const FString& MessageTime, const FString& MessageRole, const FString& MessageInfo)
{
	if (SB_ChatHistory == nullptr || MessageItemWidgetClass == nullptr) return;

	UChatMessageItem* MsgItemWidget = CreateWidget<UChatMessageItem>(this, MessageItemWidgetClass);
	if (MsgItemWidget)
	{
		MsgItemWidget->Setup(MessageTime, MessageRole, MessageInfo);
		SB_ChatHistory->AddChild(MsgItemWidget);
		SB_ChatHistory->ScrollToEnd();
	}
}

void UChatOverlay::EnableChatInput()
{
	if (ChatHistoryBackground == nullptr || ChatInput == nullptr) return;
	ChatInput->SetVisibility(ESlateVisibility::Visible);
	ChatInput->SetKeyboardFocus();
	ChatInput->SetFocus();
	ChatHistoryBackground->SetRenderOpacity(.6f);
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(ChatInput->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(true);
	}
}

void UChatOverlay::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// will be called when setFocus
	if (CommitMethod == ETextCommit::OnEnter)
	{
		FString Message = Text.ToString();
		if (!Message.IsEmpty())
		{
			if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(GetOwningPlayer()))
			{
				BlasterPlayerController->ServerSendMessage(Message);
			}
			ChatInput->SetText(FText::GetEmpty());
		}
	}
}

FReply UChatOverlay::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ChatHistoryBackground->SetRenderOpacity(.1f);
		ChatInput->SetVisibility(ESlateVisibility::Collapsed);
		bIsChatting = false;
		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->bShowMouseCursor = false;
		}
		return FReply::Handled(); 
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}
