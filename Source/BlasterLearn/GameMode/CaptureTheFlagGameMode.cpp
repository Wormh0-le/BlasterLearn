// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"

#include "BlasterLearn/GameState/BlasterGameState.h"
#include "BlasterLearn/Zone/FlagZone.h"

ACaptureTheFlagGameMode::ACaptureTheFlagGameMode()
{
	bTeamsMatch = true;
	bCaptureMatch = true;
}

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter,
                                               ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::ZoneCaptured(AFlagZone* CapturedZone)
{
	if (CapturedZone == nullptr)	return;
	BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(GameState) : BlasterGameState;
	if (BlasterGameState)
	{
		if (CapturedZone->bBaseZone)
		{
			if (CapturedZone->GetOwnerTeam() == ETeam::ET_BlueTeam)
			{
				BlasterGameState->BlueTeamScores();	
			}
			if (CapturedZone->GetOwnerTeam() == ETeam::ET_RedTeam)
			{
				BlasterGameState->RedTeamScores();
			}
			DelayedCooldownMatch();
		}
	}
}

