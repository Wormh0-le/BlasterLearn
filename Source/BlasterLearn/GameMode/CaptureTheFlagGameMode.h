// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
	
public:
	ACaptureTheFlagGameMode();
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	void ZoneCaptured(class AFlagZone* CapturedZone);
};
