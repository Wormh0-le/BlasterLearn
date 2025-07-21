// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterLearn/BlasterTypes/KillEventInfo.h"
#include "BlasterGameState.generated.h"


/**
 * 
 */
UCLASS()
class BLASTERLEARN_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated)
	TArray<class ABlasterPlayerState*> TopScoringPlayers;

	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);

	// Broadcasts a message to all players in the game
	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastMessage(const FString& MessageRole, const FString& MessageInfo);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastKillEvent(const FKillEventMessage& KillEventMessage);


	/* 
	* Teams
	*/

	void RedTeamScores();
	void BlueTeamScores();

	UPROPERTY()
	TArray<ABlasterPlayerState*> RedTeam;

	UPROPERTY()
	TArray<ABlasterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
private:
	float TopScore = 0.f;
};
