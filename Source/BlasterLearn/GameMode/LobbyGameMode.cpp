// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "XMultiPlayerSessionsSubsystem.h"
#include "GameFrameWork/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UXMultiPlayerSessionsSubsystem* OnlineSubsystem = GameInstance->GetSubsystem<UXMultiPlayerSessionsSubsystem>();
		check(OnlineSubsystem);
		if (NumberOfPlayers == OnlineSubsystem->DesiredNumPublicConnections) {
			UWorld* World = GetWorld();
			if (World) {
				bUseSeamlessTravel = true;
				FString MatchType = OnlineSubsystem->DesiredMatchType;
				if (MatchType == "Solo")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}
				else if (MatchType == "Team")
				{
					World->ServerTravel(FString("/Game/Maps/Teams?listen"));
				}
				else if (MatchType == "CaptureTheZone")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureTheFlag?listen"));
				}
			}
		}
	}
	

}
