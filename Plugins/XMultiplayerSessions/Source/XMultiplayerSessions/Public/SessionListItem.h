// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionListItem.generated.h"

/**
 * 
 */
UCLASS()
class XMULTIPLAYERSESSIONS_API USessionListItem : public UUserWidget
{
	GENERATED_BODY()

public:
	void SessionListItemSetup(FString& HostUserNameStr, FString& UserCountStr, FString& LatencyStr, TSharedPtr<FOnlineSessionSearchResult> SessionResult);

protected:
	virtual bool Initialize() override;

	virtual void NativeDestruct() override;

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Join;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HostUserName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UserCount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Latency;

	UFUNCTION()
	void JoinButtonClicked();

	// The subsystem designed to handle all online session functionality
	class UXMultiPlayerSessionsSubsystem* MultiPlayerSessionsSubsystem;

	TSharedPtr<FOnlineSessionSearchResult> SessionResult;
	
};
