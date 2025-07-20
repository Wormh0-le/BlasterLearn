// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"

#include "BlasterLearn/Character/BlasterCharacter.h"
#include "BlasterLearn/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::HandleScore()
{
	ConsecutiveScore++;
	double CurrentScoreTime = GetWorld()->GetTimeSeconds();
	if (CurrentScoreTime - LastScoreTime <= ComboScoreWindow || LastScoreTime == 0)
	{
		ComboScore++;
		ComboScoreWindow += 2; // Increment the window for the next combo
	} else
	{
		ComboScore = 1;
	}
	LastScoreTime = CurrentScoreTime;
	
}

EKillEventType ABlasterPlayerState::GetConsecutiveKillType()
{
	EKillEventType ConsecutiveKillType = EKillEventType::Default;
	if (ConsecutiveScore >= 8)
	{
		ConsecutiveKillType = EKillEventType::Legendary;
	}
	if (ConsecutiveScore == 7)
	{
		ConsecutiveKillType = EKillEventType::Godlike;
	}
	if (ConsecutiveScore == 6)
	{
		ConsecutiveKillType = EKillEventType::Dominating;
	}
	if (ConsecutiveScore == 5)
	{
		ConsecutiveKillType = EKillEventType::Unstoppable;
	}
	if (ConsecutiveScore == 4)
	{
		ConsecutiveKillType = EKillEventType::Rampage;
	}
	if (ConsecutiveScore == 3)
	{
		ConsecutiveKillType = EKillEventType::KillingSpree;
	} 
	return ConsecutiveKillType;
}

EKillEventType ABlasterPlayerState::GetComboKillType()
{
	EKillEventType ComboKillType = EKillEventType::Default;
	if (ComboScore == 2)
	{
		ComboKillType = EKillEventType::DoubleKill;
	}
	if (ComboScore == 3)
	{
		ComboKillType = EKillEventType::TripleKill;
	}
	if (ComboScore == 4)
	{
		ComboKillType = EKillEventType::QuadraKill;
	}
	if (ComboScore >= 5)
	{
		ComboKillType = EKillEventType::PentaKill;
	}
	return ComboKillType; 
}

void ABlasterPlayerState::ResetScoreStat()
{
	ConsecutiveScore = 0;
	ComboScore = 0;
	LastScoreTime = 0.0;
	ComboScoreWindow = 0.0;
}
