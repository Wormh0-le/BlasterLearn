// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTERLEARN_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();
	friend class ABlasterCharacter;
	void Heal(float HealAmount, float HealingTime);
	void Replenish(float ShieldAmount, float ReplenishTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialJumpVelocity(float Velocity);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpVelocity(float JumpVelocity);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBuffSpeed(float BaseSpeed, float CrouchSpeed);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ReplenishRampUp(float DeltaTime);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY(EditDefaultsOnly)
	float MaxBaseSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly)
	float MaxCrouchSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly)
	float MaxJumpVelocity = 3200.f;

	// Health buff
	bool bHealing = false;
	float HealingRate = 0;
	float AmountToHeal = 0.f;

	// shield buff
	bool bReplenish = false;
	float ReplenishRate = 0;
	float AmountToReplenish = 0.f;

	// Speed buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	// Jump buff
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;
};
