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
	// red progress incr
	EZS_BlueActivating	UMETA(DisplayName = "Blue Activating"),
	// red progress decr
	EZS_BlueDeactivating	UMETA(DisplayName = "Blue Deactivating"),
	// red progress pause
	EZS_BlueBlocked	UMETA(DisplayName = "Blue Blocked"),
	// blue progress incr
	EZS_RedActivating	UMETA(DisplayName = "Red Activating"),
	// blue progress decr
	EZS_RedDeactivating	UMETA(DisplayName = "Red Deactivating"),
	// blue progress pause
	EZS_RedBlocked	UMETA(DisplayName = "Red Blocked"),
};

UCLASS()
class BLASTERLEARN_API AFlagZone : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlagZone();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	class UDualBarStatusWidget* ZoneStatusWidget;

	UPROPERTY(EditDefaultsOnly)
	FString ZoneName;

	UPROPERTY(EditDefaultsOnly)
	bool bBaseZone;

	void TeleportCooldown();

	void HealCooldown();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void PollInit();
	
	UFUNCTION()
	virtual void OnAreaBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnAreaBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	
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
		int32 OtherBodyIndex
	);
private:

	UPROPERTY(EditAnywhere)
	class UBoxComponent* ZoneAreaBox;
	
	UPROPERTY(EditAnywhere)
	class USphereComponent* ZoneSphere;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ZoneMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UParticleSystemComponent* ZoneStatusEffectComponent;

	UPROPERTY(EditAnywhere)
	ETeam OwnerTeam = ETeam::ET_NoTeam;

	UPROPERTY(EditAnywhere)
	EZoneStatus ZoneStatus = EZoneStatus::EZS_Unoccupied;

	UPROPERTY(EditAnywhere)
	class UWidgetComponent* ZoneStatusWidgetComponent;

	UPROPERTY()
	class ACaptureTheFlagGameMode* CaptureGameMode;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor InitialColor;

	UPROPERTY(EditDefaultsOnly)
	float CaptureTimeCost;

	int32 TeamBluePlayerInArea = 0;
	int32 TeamRedPlayerInArea = 0;
	float CurrentCaptureTime = 0;

	void HandleZoneStatus();

	void UpdateZoneStatusBar(float DeltaTime);

	UPROPERTY(Replicated)
	bool bHealCooldown = false;

	UPROPERTY(Replicated)
	bool bTeleportCooldown = false;

	UPROPERTY(EditDefaultsOnly)
	float TeleportCoolDownTime = 5.f;

	UPROPERTY(EditDefaultsOnly)
	float HealCoolDownTime = 20.f;

	FTimerHandle HealSkillTimer;

	FTimerHandle TeleportSkillTimer;

public:
	FORCEINLINE ETeam GetOwnerTeam() const { return OwnerTeam; }
	FORCEINLINE bool IsTeleportCooldown() const { return bTeleportCooldown; }
	FORCEINLINE bool IsHealCooldown() const { return bHealCooldown; }
};
