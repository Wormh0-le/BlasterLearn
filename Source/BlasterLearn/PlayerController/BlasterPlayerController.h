// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterLearn/BlasterTypes/KillEventInfo.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bHighPing);

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);

	void ShowMessage(const FString& MessageTime, const FString& MessageRole, const FString& MessageInfo, const FLinearColor& RoleColor);

	void BroadcastKillEventMessage(const FKillEventMessage& KillEventMessage);

	UFUNCTION()
	void ToggleChat();
	
	UFUNCTION(Server, Reliable)
	void ServerSendMessage(const FString& MessageInfo);
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void CheckTimeSync(float DeltaTime);

	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; //synced with server clock as soon as possible when client is ready
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;
protected:
	virtual void BeginPlay() override;
	void CheckPing(float DeltaTime);
	void SetHUDTime();
	void PollInit();
	virtual void SetupInputComponent() override;

	// sync time between client and server

	// Requests the current server time, passing in the client's time when the request was sent, client -> server
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime, server -> client
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);
	
	float ClientServerDelta = 0.f; // difference between client and server time
	
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown, bool bTeamsMatch);

	void HighPingWarning();

	void StopHighPingWarning();

	void ShowReturnToMainMenu();

	FString GetInfoText(const TArray<class ABlasterPlayerState*>& Players);
	FString GetTeamsInfoText();
	
	bool bShowTeamScores = false;
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	UPROPERTY()
	class ABlasterGameState* BlasterGameState;

	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false;

	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false;

	float HUDScore;
	bool bInitializeScore = false;
	
	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
	
	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	float HighPingRunningTime = 0.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
	
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

	bool bReturnToMainMenuOpen = false;
	
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuClass;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenuWidget;
};
