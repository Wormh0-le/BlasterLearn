// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatMessageItem.h"
#include "Components/TextBlock.h"

void UChatMessageItem::Setup(const FString& MessageTime, const FString& MessageRole, const FString& MessageInfo, const FLinearColor& RoleColor)
{
	if (TimeText)
	{
		TimeText->SetText(FText::FromString(MessageTime));
	}
	if (RoleText)
	{
		RoleText->SetText(FText::FromString(MessageRole + ":"));
		RoleText->SetColorAndOpacity(RoleColor);
	}
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(MessageInfo));
	}
}
