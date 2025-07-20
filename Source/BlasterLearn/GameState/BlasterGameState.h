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

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastMessage(const FString& MessageRole, const FString& MessageInfo);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastKillEvent(const FKillEventMessage& KillEventMessage);

private:
	float TopScore = 0.f;
};
