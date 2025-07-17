// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatOverlay.h"

#include "ChatMessageItem.h"
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
