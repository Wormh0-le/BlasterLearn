// Fill out your copyright notice in the Description page of Project Settings.


#include "XMultiPlayerSessionsSubsystem.h"
#include <functional>
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"


UXMultiPlayerSessionsSubsystem::UXMultiPlayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnjoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	OnlineSubsystem = OnlineSubsystem == nullptr ? Online::GetSubsystem(GetWorld()) : OnlineSubsystem;
	if (OnlineSubsystem) {
		SessionInterfaceWeakPtr = OnlineSubsystem->GetSessionInterface();
	}
}

void UXMultiPlayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, const FString& MatchType, const FString& SearchKey)
{
	DesiredNumPublicConnections = NumPublicConnections;
	DesiredMatchType = MatchType;
	if (!SessionInterfaceWeakPtr.IsValid())
	{
		return;
	}
	IOnlineSessionPtr SessionInterface = SessionInterfaceWeakPtr.Pin();
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		bCreateSessionOnDestroy = true;
		DestroySession();
	}
	// store the delegate in a FDelegateHandle so we can later remove it from the delegate list
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = OnlineSubsystem->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("MatchType"), SearchKey, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		// Broadcast our own custom delegate
		MultiPlayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UXMultiPlayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!SessionInterfaceWeakPtr.IsValid())
	{
		return;
	}
	IOnlineSessionPtr SessionInterface = SessionInterfaceWeakPtr.Pin();
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	LastSessionSearchSettings = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearchSettings->MaxSearchResults = MaxSearchResults;
	LastSessionSearchSettings->bIsLanQuery = OnlineSubsystem->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearchSettings.ToSharedRef())) {
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		MultiPlayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UXMultiPlayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterfaceWeakPtr.IsValid()) {
		MultiPlayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	IOnlineSessionPtr SessionInterface = SessionInterfaceWeakPtr.Pin();
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate); 
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult)) {
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiPlayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UXMultiPlayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterfaceWeakPtr.IsValid()) {
		MultiPlayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	IOnlineSessionPtr SessionInterface = SessionInterfaceWeakPtr.Pin();

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	
	if (!SessionInterface->DestroySession(NAME_GameSession)) {
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiPlayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UXMultiPlayerSessionsSubsystem::StartSession()
{
}

void UXMultiPlayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterfaceWeakPtr.IsValid())
	{
		SessionInterfaceWeakPtr.Pin()->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiPlayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UXMultiPlayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterfaceWeakPtr.IsValid()) {
		SessionInterfaceWeakPtr.Pin()->ClearOnCancelFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	if (LastSessionSearchSettings->SearchResults.Num() <= 0){
		MultiPlayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	MultiPlayerOnFindSessionsComplete.Broadcast(LastSessionSearchSettings->SearchResults, bWasSuccessful);
}

void UXMultiPlayerSessionsSubsystem::OnjoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterfaceWeakPtr.IsValid()) {
		SessionInterfaceWeakPtr.Pin()->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiPlayerOnJoinSessionComplete.Broadcast(Result);
}

void UXMultiPlayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterfaceWeakPtr.IsValid()) {
		SessionInterfaceWeakPtr.Pin()->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bWasSuccessful && bCreateSessionOnDestroy) {
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType, LastSearchKey);
	}

	MultiPlayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UXMultiPlayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
