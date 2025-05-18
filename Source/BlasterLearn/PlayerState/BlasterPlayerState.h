// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	void AddToScore(float ScoreAmount);
	virtual void OnRep_Score() override;

private:
	class ABlasterCharacter* Character;
	class ABlasterPlayerController* Controller;
};
