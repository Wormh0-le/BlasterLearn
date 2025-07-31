// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "BlasterLearn/Character/BlasterCharacter.h"
#include "BlasterLearn/HUD/BlasterHUD.h"
#include "BlasterLearn/HUD/CharacterOverlay.h"
#include "BlasterLearn/HUD/Announcement.h"
#include "BlasterLearn/GameMode/BlasterGameMode.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterLearn/GameState/BlasterGameState.h"
#include "BlasterLearn/HUD/BannerWidget.h"
#include "BlasterLearn/HUD/ChatWidget.h"
#include "BlasterLearn/HUD/ReturnToMainMenu.h"
#include "BlasterLearn/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}
 
void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, ServerCooldownStartTime);
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			// UE_LOG(LogTemp, Warning, TEXT("Ping: %d"), PlayerState->GetCompressedPing() * 4);
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying =
		BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingAnimation &&
		BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())	return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController()) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	} else if (MatchState == MatchState::Cooldown) {
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode)
		{
			ServerCooldownStartTime = BlasterGameMode->CooldownStartingTime;
		}
		HandleCooldown();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	if (BlasterGameMode) {
		WarmupTime = BlasterGameMode->WarmupTime;
		MatchTime = BlasterGameMode->MatchTime;
		LevelStartingTime = BlasterGameMode->LevelStartingTime;
		CooldownTime = BlasterGameMode->CooldownTime;
		MatchState = BlasterGameMode->GetMatchState();
		bShowTeamScores = BlasterGameMode->bTeamsMatch;
		// OnMatchStateSet(MatchState);
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime, bShowTeamScores);
		/*if (BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnnouncement();
		}*/
	}
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown, bool bTeamsMatch)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	CooldownTime = Cooldown;
	MatchState = StateOfMatch;
	bShowTeamScores = bTeamsMatch;
	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	} else if (MatchState == MatchState::Cooldown) {
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if(BlasterHUD->CharacterOverlay == nullptr) {
			BlasterHUD->AddCharacterOverlay();
		}
		if (BlasterHUD->Announcement) {
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		InitTeamScores();
		if (!bShowTeamScores)
		{
			HideTeamScores();	
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD) {
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = BlasterHUD->Announcement && BlasterHUD->Announcement->AnnouncementText && BlasterHUD->Announcement->InfoText;
		if (bHUDValid) {
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Start In:");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			
			BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
			if (BlasterGameState) {
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText() : GetInfoText(TopPlayers);
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter)
	{
		BlasterCharacter->FireButtonReleased();
		BlasterCharacter->bDisableGameplay = true;
	}
}

FString ABlasterPlayerController::GetInfoText(const TArray<ABlasterPlayerState*>& Players)
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == BlasterPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString ABlasterPlayerController::GetTeamsInfoText()
{
	BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
	if (BlasterGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr) {
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeGrenades) SetHUDGrenades(HUDGrenades);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo)	SetHUDWeaponAmmo(HUDWeaponAmmo);
			}
		}
	}
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr)	return;
	InputComponent->BindAction("Quit", IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);
	InputComponent->BindAction("ToggleChat", IE_Pressed, this, &ABlasterPlayerController::ToggleChat);
}

void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->HighPingImage && 
		BlasterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid) {
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation, 0.f, 5);
	}
}

void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->HighPingImage && 
		BlasterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid) {
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);	
		}
	}
}

void ABlasterPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuClass == nullptr)	return;
	if (ReturnToMainMenuWidget == nullptr)
	{
		ReturnToMainMenuWidget = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuClass);
	}
	if (ReturnToMainMenuWidget)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenuWidget->MenuSetup();
		}
		else
		{
			ReturnToMainMenuWidget->MenuTeardown();	
		}
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) {
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	} else if (MatchState == MatchState::InProgress) {
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	} else if (MatchState == MatchState::Cooldown) {
		TimeLeft = ServerCooldownStartTime == 0.f ? CooldownTime : CooldownTime - (GetServerTime() - ServerCooldownStartTime);
	}
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	if (HasAuthority()) {
		if (BlasterGameMode == nullptr)
		{
			BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
			LevelStartingTime = BlasterGameMode->LevelStartingTime;
		}
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	if (CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown) {
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress) {
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->HealthBar && 
		BlasterHUD->CharacterOverlay->HealthText;
	if (bHUDValid) {
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->ShieldBar && 
		BlasterHUD->CharacterOverlay->ShieldText;
	if (bHUDValid) {
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	} else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	} else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades) {
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	} else {
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void ABlasterPlayerController::HideTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore &&
		BlasterHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->RedTeamScore->SetRenderOpacity(0);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetRenderOpacity(0);
	}
}

void ABlasterPlayerController::InitTeamScores()
{
	SetHUDRedTeamScore(0);
	SetHUDBlueTeamScore(0);
}

void ABlasterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::ShowMessage(const FString& MessageTime, const FString& MessageRole, const FString& MessageInfo, const FLinearColor& RoleColor)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ChatBoard;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->ChatBoard->AddMessage(MessageTime, MessageRole, MessageInfo, RoleColor);
	}
}

void ABlasterPlayerController::BroadcastKillEventMessage(const FKillEventMessage& KillEventMessage)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->KillBanner;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->KillBanner->KillEventQueue.Enqueue(KillEventMessage);
		BlasterHUD->CharacterOverlay->KillBanner->KillEventCount.Increment();
		BlasterHUD->DisplayKillEvent();
	}
}

void ABlasterPlayerController::ToggleChat()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ChatBoard;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->ChatBoard->EnableChatInput();
	}
}

void ABlasterPlayerController::ServerSendMessage_Implementation(const FString& MessageInfo)
{
	BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterGameState && BlasterPlayerState) {
		BlasterGameState->MulticastBroadcastMessage(BlasterPlayerState->GetPlayerName(), MessageInfo, BlasterPlayerState->GetTeam());
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f) {
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60.f;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && 
		BlasterHUD->Announcement && 
		BlasterHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f) {
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60.f;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}



