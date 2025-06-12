// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "BlasterLearn/PlayerState/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
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
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = PlayerScore;
	}
}
