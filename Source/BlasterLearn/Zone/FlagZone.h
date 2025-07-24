// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterLearn/BlasterTypes/Team.h"
#include "GameFramework/Actor.h"
#include "FlagZone.generated.h"


UENUM(BlueprintType)
enum class EZoneStatus: uint8
{
	EZS_Unoccupied	UMETA(DisplayName = "Unoccupied"),
	EZS_Occupied	UMETA(DisplayName = "Occupied"),
	EZS_Activating	UMETA(DisplayName = "Activating"),
	EZS_Deactivating	UMETA(DisplayName = "Deactivating"),
};

UCLASS()
class BLASTERLEARN_API AFlagZone : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlagZone();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* ZoneSphere;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ZoneMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UParticleSystemComponent* ZoneStatusEffectComponent;

	UPROPERTY(EditAnywhere, Replicated)
	ETeam OwnerTeam = ETeam::ET_NoTeam;

	UPROPERTY(EditAnywhere, Replicated)
	EZoneStatus ZoneStatus = EZoneStatus::EZS_Unoccupied;

	UPROPERTY(EditAnywhere)
	class UDualBarStatusWidget* ZoneStatusWidget;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor InitialColor;
};
