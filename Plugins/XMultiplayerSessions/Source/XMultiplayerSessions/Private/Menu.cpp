// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "XMultiPlayerSessionsSubsystem.h"
#include "SessionListItem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	SetRenderScale({0.75, 0.75});

	UWorld* World = GetWorld();
	if(World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiPlayerSessionsSubsystem = GameInstance->GetSubsystem<UXMultiPlayerSessionsSubsystem>();
	}

	if (MultiPlayerSessionsSubsystem) {
		MultiPlayerSessionsSubsystem->MultiPlayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnRefreshSessionList);
		//MultiPlayerSessionsSubsystem->MultiPlayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	if (Btn_Host)
	{
		Btn_Host->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (Btn_Refresh)
	{
		Btn_Refresh->OnClicked.AddDynamic(this, &ThisClass::RefreshButtonClicked);
	}
	return true;
}

void UMenu::NativeDestruct()
{
	Menu_TearDown();
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString(TEXT("Session created successfully."))
			);
		}

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session."))
			);
		}
		Btn_Host->SetIsEnabled(true);
	}
}

void UMenu::OnRefreshSessionList(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)  
{  
   if (MultiPlayerSessionsSubsystem == nullptr) {  
       return;  
   }  
   TitleText->SetText(FText::FromString(TEXT("房间列表"))); 
   SB_SessionList->ClearChildren();
   for (const FOnlineSessionSearchResult& Result : SessionResults) {  
       //FString SettingsValue;  
       //Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

       //if (SettingsValue == MatchType) {      
		//FString Id = Result.GetSessionIdStr();
		FString HostUserName = FString::Printf(TEXT("%s"), *Result.Session.OwningUserName);
		int32 MaxUsers = Result.Session.SessionSettings.NumPublicConnections;  
		int32 CurrentUsers = MaxUsers - Result.Session.NumOpenPublicConnections;  
		FString LatencyStr = FString::Printf(TEXT("%dms"), Result.PingInMs);  
		FString UserCountStr = FString::Printf(TEXT("%d/%d"), CurrentUsers, MaxUsers);  

		USessionListItem* SessionItem = CreateWidget<USessionListItem>(this, ItemWidgetClass);  
		if (SessionItem) {  
			TSharedPtr<FOnlineSessionSearchResult> ResultPtr = MakeShared<FOnlineSessionSearchResult>(Result);  
			SessionItem->SessionListItemSetup(HostUserName, UserCountStr, LatencyStr, ResultPtr);
			SB_SessionList->AddChild(SessionItem);  
		}  
       //}  
   }  
   if (bWasSuccessful || SessionResults.Num() == 0) {  
       Btn_Refresh->SetIsEnabled(true);  
   }
}

//void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
//{
//	if (MultiPlayerSessionsSubsystem == nullptr) {
//		return;
//	}
//	for (auto Result : SessionResults) {
//		FString Id = Result.GetSessionIdStr();
//		FString User = Result.Session.OwningUserName;
//		FString SettingsValue;
//		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
//
//		if (SettingsValue == MatchType) {	
//			MultiPlayerSessionsSubsystem->JoinSession(Result);
//			return;
//		}
//	}
//	if (!bWasSuccessful || SessionResults.Num() == 0) {
//		Btn_Join->SetIsEnabled(true);
//	}
//}

//void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
//{
//	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//	if (Subsystem) {
//		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
//		if (SessionInterface.IsValid()) {
//			FString Address;
//			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
//
//			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
//			if (PlayerController) {
//				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
//			}
//		}
//	}
//	if (Result != EOnJoinSessionCompleteResult::Success) {
//		Btn_Join->SetIsEnabled(true);
//	}
//}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	Btn_Host->SetIsEnabled(false);
	if (MultiPlayerSessionsSubsystem)
	{
		MultiPlayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::RefreshButtonClicked()
{
	Btn_Refresh->SetIsEnabled(false);
	TitleText->SetText(FText::FromString(TEXT("搜索中...")));
	if (MultiPlayerSessionsSubsystem) {
		SB_SessionList->ClearChildren();
		MultiPlayerSessionsSubsystem->FindSessions(100000);
	}
}

//void UMenu::JoinButtonClicked()
//{
//	Btn_Join->SetIsEnabled(false);
//	
//}

void UMenu::Menu_TearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
