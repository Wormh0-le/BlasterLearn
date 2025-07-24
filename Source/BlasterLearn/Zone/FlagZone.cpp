// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AFlagZone::AFlagZone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
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
	ZoneSphere->SetSphereRadius(300.f);
	ZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ZoneSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ZoneSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	ZoneStatusWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ZoneStatusBar"));
	ZoneStatusWidget->SetupAttachment(RootComponent);
	
}

void AFlagZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFlagZone, OwnerTeam);
	DOREPLIFETIME(AFlagZone, ZoneStatus);
}

// Called when the game starts or when spawned
void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void AFlagZone::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

