// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterLearn/BlasterTypes/KillEventInfo.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
	virtual void OnRep_Score() override;
	
	UFUNCTION()
	virtual void OnRep_Defeats();

	void HandleScore();

	EKillEventType GetConsecutiveKillType();

	EKillEventType GetComboKillType();

	void ResetScoreStat();
private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;
	
	int32 ConsecutiveScore = 0;

	int32 ComboScore = 0;

	double LastScoreTime = 0.0;

	double ComboScoreWindow = 0.0;
public:
	FORCEINLINE int32 GetConsecutiveScore() const { return ConsecutiveScore; }
};
