// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "BlasterLearn/GameState/BlasterGameState.h"
#include "BlasterLearn/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"


void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
	if (BlasterGameState)
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BlasterGameState->BlueTeam.Num() >= BlasterGameState->RedTeam.Num())
			{
				BlasterGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BlasterGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
	if (BlasterGameState)
	{
		ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
		if (BPState)
		{
			switch (BPState->GetTeam())
			{
			case ETeam::ET_RedTeam:
				BlasterGameState->RedTeam.Remove(BPState);
				break;
			case ETeam::ET_BlueTeam:
				BlasterGameState->BlueTeam.Remove(BPState);
				break;
			default:
				break;
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* AttackerController, AController* VictimController, float BaseDamage, bool bFriendlyFire)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	if (AttackerPlayerState == nullptr || VictimPlayerState == nullptr || bFriendlyFire) return BaseDamage;
	if (AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
	if (BlasterGameState)
	{
		for (auto PState : BlasterGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BlasterGameState->BlueTeam.Num() >= BlasterGameState->RedTeam.Num())
				{
					BlasterGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BlasterGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
