// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "BlasterLearn/Character/BlasterCharacter.h"
#include "BlasterLearn/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "BlasterLearn/PlayerState/BlasterPlayerState.h"


void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, AllTeleports);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	float PlayerScore = ScoringPlayer->GetScore();
	if (TopScoringPlayers.IsEmpty()) {
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = PlayerScore;
	}else if (PlayerScore == TopScore) {
		TopScoringPlayers.AddUnique(ScoringPlayer);
	} else if (PlayerScore > TopScore) {
		for (auto CurrentTopPlayerState : TopScoringPlayers)
		{
			ABlasterCharacter* CurrentTopCharacter = Cast<ABlasterCharacter>(CurrentTopPlayerState->GetPawn());
			if (CurrentTopCharacter)
			{
				CurrentTopCharacter->MulticastLostTheLead();
			}
		}
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = PlayerScore;
	}
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::MulticastBroadcastKillEvent_Implementation(const FKillEventMessage& KillEventMessage)
{
	for (APlayerState* PlayerState : PlayerArray)
	{
		if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(PlayerState->GetPlayerController()))
		{
			if (BlasterPlayerController->IsLocalPlayerController())
			{
				BlasterPlayerController->BroadcastKillEventMessage(KillEventMessage);
			}
		}
	}
}

void ABlasterGameState::MulticastBroadcastMessage_Implementation(const FString& MessageRole, const FString& MessageInfo, ETeam MessageTeam, bool bPublic)
{
	for (APlayerState* PlayerState : PlayerArray)
	{
		ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(PlayerState);
		FLinearColor RoleColor = FLinearColor::Green;
		if (MessageTeam != ETeam::ET_NoTeam)
		{
			if (!bPublic && BlasterPlayerState->GetTeam() != MessageTeam)	continue;
			if (bPublic)
			{
				if (MessageTeam == ETeam::ET_BlueTeam)
				{
					RoleColor = FLinearColor::Blue;
				}
				if (MessageTeam == ETeam::ET_RedTeam)
				{
					RoleColor = FLinearColor::Red;
				}
			}
		}
		if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(PlayerState->GetPlayerController()))
		{
			if (BlasterPlayerController->IsLocalPlayerController())
			{
				FDateTime CurrentTime = FDateTime::Now();
				FString TimeString = CurrentTime.ToString(TEXT("[%y-%m-%d %H:%M:%S]"));
				BlasterPlayerController->ShowMessage(TimeString, MessageRole, MessageInfo, RoleColor);
			}
		}
	}
}
