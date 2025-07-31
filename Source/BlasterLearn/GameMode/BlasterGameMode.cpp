// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "BlasterLearn/Character/BlasterCharacter.h"
#include "BlasterLearn/PlayerController/BlasterPlayerController.h"
#include "BlasterLearn/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFrameWork/PlayerStart.h"
#include "BlasterLearn/GameState/BlasterGameState.h"
#include "BlasterLearn/PlayerStart/TeamPlayerStart.h"
#include "BlasterLearn/Zone/FlagZone.h"


namespace MatchState {
	const FName Cooldown = FName("Cooldown");	
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	} else if (MatchState == MatchState::InProgress) {
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f) {
			SetMatchState(MatchState::Cooldown);
		}
	} else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime - (GetWorld()->GetTimeSeconds() - CooldownStartingTime);
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::InitGameState()
{
	Super::InitGameState();
	MapInitialized();
}

void ABlasterGameMode::MapInitialized()
{
	BlasterGameState = BlasterGameState == nullptr ? GetGameState<ABlasterGameState>() : BlasterGameState;
	if (BlasterGameState)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		BlasterGameState->PlayerStarts = PlayerStarts;
		if (bTeamsMatch)
		{
			for (auto Start : PlayerStarts)
			{
				ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
				if (TeamStart && TeamStart->Team == ETeam::ET_BlueTeam)
				{
					BlasterGameState->TeamBluePlayerStarts.AddUnique(TeamStart);
				}
				if (TeamStart && TeamStart->Team == ETeam::ET_RedTeam)
				{
					BlasterGameState->TeamRedPlayerStarts.AddUnique(TeamStart);
				}
			}
		}
		if (bCaptureMatch)
		{
			TArray<AActor*> FlagZones;
			UGameplayStatics::GetAllActorsOfClass(this, AFlagZone::StaticClass(), FlagZones);
			for (auto Zone : FlagZones)
			{
				AFlagZone* FlagZonePtr = Cast<AFlagZone>(Zone);
				if (FlagZonePtr && !FlagZonePtr->bBaseZone)
				{
					BlasterGameState->AllTeleports.AddUnique(FlagZonePtr);
				}
			}
		}
	}
}

float ABlasterGameMode::CalculateDamage(AController* AttackerController, AController* VictimController, float BaseDamage, bool bFriendlyFire)
{
	return BaseDamage;
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	if (MatchState == MatchState::Cooldown)
	{
		CooldownStartingTime = GetWorld()->GetTimeSeconds();
	}
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr)	return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	
	BlasterGameState = BlasterGameState == nullptr ? GetGameState<ABlasterGameState>() : BlasterGameState;
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		AttackerPlayerState->HandleScore();
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
		if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterCharacter* AttackerCharacter = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if (AttackerCharacter)
			{
				AttackerCharacter->MulticastGainTheLead();
			}
		}
		FKillEventMessage KillEventMessage(AttackerPlayerState->GetPlayerName(), VictimPlayerState->GetPlayerName(), EKillEventType::Kill);
		if (VictimPlayerState->GetConsecutiveScore() >= 3 && AttackerPlayerState->GetConsecutiveScore() < 1)
		{
			KillEventMessage.KillType = EKillEventType::Shutdown;
		}
		BlasterGameState->MulticastBroadcastKillEvent(KillEventMessage);
		// Check for consecutive kills and combo kills
		EKillEventType ConsecutiveKillType = AttackerPlayerState->GetConsecutiveKillType();
		EKillEventType ComboKillType = AttackerPlayerState->GetComboKillType();
		if (ComboKillType != EKillEventType::Default)
		{
			FKillEventMessage ComboKillEventMessage(AttackerPlayerState->GetPlayerName(), VictimPlayerState->GetPlayerName(), ComboKillType);
			BlasterGameState->MulticastBroadcastKillEvent(ComboKillEventMessage);
		}
		if (ConsecutiveKillType != EKillEventType::Default)
		{
			FKillEventMessage ConsecutiveKillEventMessage(AttackerPlayerState->GetPlayerName(), VictimPlayerState->GetPlayerName(), ConsecutiveKillType);
			BlasterGameState->MulticastBroadcastKillEvent(ConsecutiveKillEventMessage);
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
		VictimPlayerState->ResetScoreStat();
	}
	
	if (ElimmedCharacter) {
		ElimmedCharacter->MulticastLostTheLead();
		ElimmedCharacter->Elim(false);
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter) {
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	BlasterGameState = BlasterGameState == nullptr ? GetGameState<ABlasterGameState>() : BlasterGameState;
	if (ElimmedController && BlasterGameState) {
		int32 Selection;
		if (bTeamsMatch)
		{
			ETeam ElimmedTeam = ElimmedController->GetPlayerState<ABlasterPlayerState>()->GetTeam();
			if (ElimmedTeam == ETeam::ET_BlueTeam)
			{
				Selection = FMath::RandRange(0, BlasterGameState->TeamBluePlayerStarts.Num() - 1);
				RestartPlayerAtPlayerStart(ElimmedController, BlasterGameState->TeamBluePlayerStarts[Selection]);	
			}
			if (ElimmedTeam == ETeam::ET_RedTeam)
			{
				Selection = FMath::RandRange(0, BlasterGameState->TeamRedPlayerStarts.Num() - 1);
				RestartPlayerAtPlayerStart(ElimmedController, BlasterGameState->TeamRedPlayerStarts[Selection]);
			}
		}
		else
		{
			Selection = FMath::RandRange(0, BlasterGameState->PlayerStarts.Num() - 1);
			RestartPlayerAtPlayerStart(ElimmedController, BlasterGameState->PlayerStarts[Selection]);	
		}
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* LeavingPlayerState)
{
	if (LeavingPlayerState == nullptr) return;
	BlasterGameState = BlasterGameState == nullptr ? GetGameState<ABlasterGameState>() : BlasterGameState;
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(LeavingPlayerState))
	{
		BlasterGameState->TopScoringPlayers.Remove(LeavingPlayerState);
	}
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(LeavingPlayerState->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
	if (BlasterGameState)
	{
		FString ElimMessage = FString::Printf(TEXT("%s leave the match"), *LeavingPlayerState->GetPlayerName());
		BlasterGameState->MulticastBroadcastMessage("System", ElimMessage);
	}
}
