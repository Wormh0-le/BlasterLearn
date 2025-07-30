// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class XMULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup();

protected:
	virtual bool Initialize() override;

	virtual void NativeDestruct() override;

	// Callbacks for the custom delegates on the MultiplayerSessionSubsystem
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnRefreshSessionList(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

	//void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Host;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Refresh;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* SB_SessionList;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* CB_PlayerNum;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* CB_MatchType;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void RefreshButtonClicked();

	void Menu_TearDown();
	
	// The subsystem designed to handle all online session functionality
	UPROPERTY()
	class UXMultiPlayerSessionsSubsystem* MultiPlayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("Solo")};
	FString SearchKey{TEXT("Wormh01e")};
	FString PathToLobby{TEXT("/Game/Maps/Lobby?listen")};

	UPROPERTY(EditAnywhere, Category = "ScrollItemSetting")
	TSubclassOf<UUserWidget> ItemWidgetClass;
};
