// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterLearn/BlasterTypes/KillEventInfo.h"
#include "BlasterLearn/BlasterTypes/Team.h"
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

	UPROPERTY(Replicated)
	TArray<class AFlagZone*> AllTeleports;

	// TODO: set by datable, only need location and rotation
	UPROPERTY(Replicated)
	TArray<AActor*> PlayerStarts;

	UPROPERTY(Replicated)
	TArray<class ATeamPlayerStart*> TeamBluePlayerStarts;

	UPROPERTY(Replicated)
	TArray<ATeamPlayerStart*> TeamRedPlayerStarts;

	// Broadcasts a message to all players in the game
	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastMessage(const FString& MessageRole, const FString& MessageInfo, ETeam MessageTeam = ETeam::ET_NoTeam, bool bPublic = false);

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
