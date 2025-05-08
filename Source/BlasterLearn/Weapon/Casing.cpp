// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
    CasingMesh->SetNotifyRigidBodyCollision(true);
    CountdownTime = .15f;
	ShellEjectionImpulse = 7.f;
}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (ShellSound) {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ShellSound,
            GetActorLocation()
        );
    }
    FTimerHandle DestroyTimerHandle;
    // Start a countdown before destroying the casing
    GetWorldTimerManager().SetTimer(
        DestroyTimerHandle, 
        this, 
        &ACasing::DestroyCasing, 
        CountdownTime, 
        false
    );
}

void ACasing::DestroyCasing()
{
    Destroy();
}

