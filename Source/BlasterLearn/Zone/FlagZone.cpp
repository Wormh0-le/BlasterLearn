// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"

#include "BlasterLearn/Character/BlasterCharacter.h"
#include "BlasterLearn/GameMode/CaptureTheFlagGameMode.h"
#include "BlasterLearn/GameState/BlasterGameState.h"
#include "BlasterLearn/HUD/DualBarStatusWidget.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AFlagZone::AFlagZone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ZoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneMesh"));
	ZoneMesh->SetupAttachment(RootComponent);
	SetRootComponent(ZoneMesh);

	ZoneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ZoneMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	ZoneStatusEffectComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ZoneStatusEffect"));
	ZoneStatusEffectComponent->SetupAttachment(RootComponent);

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	ZoneSphere->SetupAttachment(RootComponent);
	ZoneSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZoneSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ZoneSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	ZoneAreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneAreaBox"));
	ZoneAreaBox->SetupAttachment(RootComponent);
	ZoneAreaBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ZoneAreaBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ZoneAreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	ZoneStatusWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ZoneStatusWidgetComponent"));
	ZoneStatusWidgetComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
}

void AFlagZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFlagZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PollInit();
	HandleZoneStatus();
	UpdateZoneStatusBar(DeltaTime);
}

void AFlagZone::PollInit()
{
	if (ZoneStatusWidget == nullptr)
	{
		if (UUserWidget* Widget = ZoneStatusWidgetComponent->GetUserWidgetObject())
		{
			ZoneStatusWidget = Cast<UDualBarStatusWidget>(Widget);
			if (ZoneStatusWidget)
			{
				if (ZoneStatus == EZoneStatus::EZS_Occupied)
				{
					ZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
					if (OwnerTeam == ETeam::ET_BlueTeam)
					{
						ZoneStatusWidget->SetRedProgress(1.f);
					}
					if (OwnerTeam == ETeam::ET_RedTeam)
					{
						ZoneStatusWidget->SetBlueProgress(1.f);
					}
				}
			}
		}
		ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
		ZoneSphere->OnComponentEndOverlap.AddDynamic(this, &AFlagZone::OnSphereEndOverlap);
		ZoneAreaBox->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnAreaBoxOverlap);
		ZoneAreaBox->OnComponentEndOverlap.AddDynamic(this, &AFlagZone::OnAreaBoxEndOverlap);
	}
}

void AFlagZone::HandleZoneStatus()
{
	if (TeamBluePlayerInArea == TeamRedPlayerInArea && TeamBluePlayerInArea == 0)	return;
	switch (ZoneStatus)
	{
	case EZoneStatus::EZS_Unoccupied:
		if (TeamBluePlayerInArea < TeamRedPlayerInArea && TeamBluePlayerInArea == 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_RedActivating;
		}
		if (TeamBluePlayerInArea > TeamRedPlayerInArea && TeamRedPlayerInArea == 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_BlueActivating;
		}
		break;
	case EZoneStatus::EZS_Occupied:
		if (TeamBluePlayerInArea < TeamRedPlayerInArea && OwnerTeam == ETeam::ET_BlueTeam && TeamBluePlayerInArea == 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_BlueDeactivating;
			CaptureGameMode = CaptureGameMode == nullptr ? GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>() : CaptureGameMode;
			if (CaptureGameMode && CaptureGameMode->BlasterGameState)
			{
				FString OccupiedMessage = FString::Printf(TEXT("TeamRed is taking over TeamBlue's point %s"), *ZoneName);
				CaptureGameMode->BlasterGameState->MulticastBroadcastMessage("System", OccupiedMessage);
			}
		}
		if (TeamBluePlayerInArea > TeamRedPlayerInArea && OwnerTeam == ETeam::ET_RedTeam && TeamRedPlayerInArea == 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_RedDeactivating;
			CaptureGameMode = CaptureGameMode == nullptr ? GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>() : CaptureGameMode;
			if (CaptureGameMode && CaptureGameMode->BlasterGameState)
			{
				FString OccupiedMessage = FString::Printf(TEXT("TeamBlue is taking over TeamRed's point %s"), *ZoneName);
				CaptureGameMode->BlasterGameState->MulticastBroadcastMessage("System", OccupiedMessage);
			}
		}
		break;
	case EZoneStatus::EZS_BlueActivating:
		if (TeamBluePlayerInArea == 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
			ZoneStatus = EZoneStatus::EZS_BlueDeactivating;
		}
		if (TeamRedPlayerInArea != 0 && TeamBluePlayerInArea != 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
			ZoneStatus = EZoneStatus::EZS_BlueBlocked;	
		}
		break;
	case EZoneStatus::EZS_BlueDeactivating:
		if (TeamBluePlayerInArea != 0 && TeamRedPlayerInArea == 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_BlueActivating;
		}
		if (TeamRedPlayerInArea != 0 && TeamBluePlayerInArea != 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
			ZoneStatus = EZoneStatus::EZS_BlueBlocked;	
		}
		break;
	case EZoneStatus::EZS_RedActivating:
		if (TeamRedPlayerInArea == 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
			ZoneStatus = EZoneStatus::EZS_RedDeactivating;
		}
		if (TeamRedPlayerInArea != 0 && TeamBluePlayerInArea != 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
			ZoneStatus = EZoneStatus::EZS_RedBlocked;	
		}
		break;
	case EZoneStatus::EZS_RedDeactivating:
		if (TeamRedPlayerInArea != 0 && TeamBluePlayerInArea == 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_RedActivating;
		}
		if (TeamRedPlayerInArea != 0 && TeamBluePlayerInArea != 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
			ZoneStatus = EZoneStatus::EZS_RedBlocked;	
		}
		break;
	case EZoneStatus::EZS_BlueBlocked:
		if (TeamRedPlayerInArea == 0 && TeamBluePlayerInArea > 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_BlueActivating;
		}
		if (TeamBluePlayerInArea == 0 && TeamRedPlayerInArea > 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_BlueDeactivating;
		}
		break;
	case EZoneStatus::EZS_RedBlocked:
		if (TeamBluePlayerInArea == 0 && TeamRedPlayerInArea > 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_RedActivating;
		}
		if (TeamRedPlayerInArea == 0 && TeamBluePlayerInArea > 0)
		{
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("Capturing")));
			ZoneStatus = EZoneStatus::EZS_RedDeactivating;
		}
		break;
	}
}

void AFlagZone::UpdateZoneStatusBar(float DeltaTime)
{
	if (ZoneStatusWidget == nullptr)	return;
	float CurRedProgress = ZoneStatusWidget->GetRedProgress();
	float CurBlueProgress = ZoneStatusWidget->GetBlueProgress();
	switch (ZoneStatus)
	{
	case EZoneStatus::EZS_BlueActivating:
		CurrentCaptureTime += DeltaTime;
		CurRedProgress = FMath::Clamp(CurRedProgress + CurrentCaptureTime / CaptureTimeCost, 0.f, 1.f);
		ZoneStatusWidget->SetRedProgress(CurRedProgress);
		if (CurRedProgress >= 1.f)
		{
			CurrentCaptureTime = 0.f;
			ZoneStatus = EZoneStatus::EZS_Occupied;
			OwnerTeam = ETeam::ET_BlueTeam;
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
			ZoneStatusEffectComponent->SetColorParameter(FName("Color"), FLinearColor::Blue);
			ZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			CaptureGameMode = CaptureGameMode == nullptr ? GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>() : CaptureGameMode;
			if (CaptureGameMode && CaptureGameMode->BlasterGameState)
			{	
				FString OccupiedMessage = FString::Printf(TEXT("TeamBlue has taken over point %s"), *ZoneName);
				CaptureGameMode->BlasterGameState->MulticastBroadcastMessage("System", OccupiedMessage);
				CaptureGameMode->ZoneCaptured(this);
			}
		}
		break;
	case EZoneStatus::EZS_BlueDeactivating:
		CurrentCaptureTime += DeltaTime;
		CurRedProgress = FMath::Clamp(CurRedProgress - CurrentCaptureTime / CaptureTimeCost, 0.f, 1.f);
		ZoneStatusWidget->SetRedProgress(CurRedProgress);
		ZoneSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (CurRedProgress == 0.f)
		{
			CurrentCaptureTime = 0.f;
			ZoneStatus = EZoneStatus::EZS_Unoccupied;
			OwnerTeam = ETeam::ET_NoTeam;
			ZoneStatusEffectComponent->SetColorParameter(FName("Color"), InitialColor);
		}
		break;
	case EZoneStatus::EZS_RedActivating:
		CurrentCaptureTime += DeltaTime;
		CurBlueProgress = FMath::Clamp(CurBlueProgress + CurrentCaptureTime / CaptureTimeCost, 0.f, 1.f);
		ZoneStatusWidget->SetBlueProgress(CurBlueProgress);
		if (CurBlueProgress >= 1.f)
		{
			CurrentCaptureTime = 0.f;
			ZoneStatus = EZoneStatus::EZS_Occupied;
			OwnerTeam = ETeam::ET_RedTeam;
			ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
			ZoneStatusEffectComponent->SetColorParameter(FName("Color"), FLinearColor::Red);
			ZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			CaptureGameMode = CaptureGameMode == nullptr ? GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>() : CaptureGameMode;
			if (CaptureGameMode && CaptureGameMode->BlasterGameState)
			{
				FString OccupiedMessage = FString::Printf(TEXT("TeamRed has taken over point %s"), *ZoneName);
				CaptureGameMode->BlasterGameState->MulticastBroadcastMessage("System", OccupiedMessage);
				CaptureGameMode->ZoneCaptured(this);
			}
		}
		break;
	case EZoneStatus::EZS_RedDeactivating:
		CurrentCaptureTime += DeltaTime;
		CurBlueProgress = FMath::Clamp(CurBlueProgress - CurrentCaptureTime / CaptureTimeCost, 0.f, 1.f);
		ZoneStatusWidget->SetBlueProgress(CurBlueProgress);
		ZoneSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (CurBlueProgress == 0.f)
		{
			CurrentCaptureTime = 0.f;
			ZoneStatus = EZoneStatus::EZS_Unoccupied;
			OwnerTeam = ETeam::ET_NoTeam;
			ZoneStatusEffectComponent->SetColorParameter(FName("Color"), InitialColor);
		}
		break;
	case EZoneStatus::EZS_Occupied:
		CurrentCaptureTime = 0.f;
	case EZoneStatus::EZS_Unoccupied:
		CurrentCaptureTime = 0.f;
	default:
		break;
	}
}

void AFlagZone::OnAreaBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && BlasterCharacter->IsHoldingTheFlag())
	{
		if (BlasterCharacter->GetTeam() == ETeam::ET_BlueTeam)
		{
			TeamBluePlayerInArea += 1;
		}
		if (BlasterCharacter->GetTeam() == ETeam::ET_RedTeam)
		{
			TeamRedPlayerInArea += 1;
		}
	}
}

void AFlagZone::OnAreaBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && BlasterCharacter->IsHoldingTheFlag())
	{
		if (BlasterCharacter->GetTeam() == ETeam::ET_BlueTeam)
		{
			TeamBluePlayerInArea -= 1;
		}
		if (BlasterCharacter->GetTeam() == ETeam::ET_RedTeam)
		{
			TeamRedPlayerInArea -= 1;
		}
	}
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// TODO: random teleport(need cooldown)
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && BlasterCharacter->GetTeam() == OwnerTeam && ZoneStatus == EZoneStatus::EZS_Occupied)
	{
		ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("E - Teleport")));
	}
}


void AFlagZone::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && BlasterCharacter->GetTeam() == OwnerTeam && ZoneStatus == EZoneStatus::EZS_Occupied)
	{
		ZoneStatusWidget->SetStatusInfo(FText::FromString(TEXT("")));
	}
}
