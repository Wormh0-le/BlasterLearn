// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterLearn/BlasterTypes/KillEventInfo.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"


namespace MatchState {
	extern BLASTERLEARN_API const FName Cooldown;	// Match duration has been reached. Display winner and begin cooldown timer.
}

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;
	
	virtual float CalculateDamage(AController* AttackerController, AController* VictimController, float BaseDamage, bool bFriendlyFire);
	
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController);

	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);

	void PlayerLeftGame(class ABlasterPlayerState* LeavingPlayerState);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;
protected:
	virtual  void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
