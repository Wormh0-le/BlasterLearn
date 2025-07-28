// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "BlasterLearn/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ReplenishRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	AmountToHeal += HealAmount;
	HealingRate = HealAmount/ HealingTime;
}

void UBuffComponent::Replenish(float ShieldAmount, float ReplenishTime)
{
	bReplenish = true;
	AmountToReplenish += ShieldAmount;
	ReplenishRate = ShieldAmount / ReplenishTime;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed())	return;
	float HealThisFrame = HealingRate * DeltaTime;
	if (Character->IsHoldingTheFlag()) HealThisFrame *= 2;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ReplenishRampUp(float DeltaTime)
{
	if (!bReplenish || Character == nullptr || Character->IsElimmed())	return;
	float ReplenishThisFrame = ReplenishRate * DeltaTime;
	if (Character->IsHoldingTheFlag()) ReplenishThisFrame *= 2;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	AmountToReplenish -= ReplenishThisFrame;

	if (AmountToReplenish <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenish = false;
		AmountToReplenish = 0.f;
	}
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr)	return;
	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeed,
		BuffTime
	);
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = FMath::Clamp(Character->GetCharacterMovement()->MaxWalkSpeed + BuffBaseSpeed, InitialBaseSpeed, MaxBaseSpeed);
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = FMath::Clamp(Character->GetCharacterMovement()->MaxWalkSpeedCrouched + BuffCrouchSpeed, InitialCrouchSpeed, MaxCrouchSpeed);
	}
	MulticastBuffSpeed(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::MulticastBuffSpeed_Implementation(float BaseSpeed, float CrouchSpeed)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = FMath::Clamp(Character->GetCharacterMovement()->MaxWalkSpeed + BaseSpeed, InitialBaseSpeed, MaxBaseSpeed);
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = FMath::Clamp(Character->GetCharacterMovement()->MaxWalkSpeedCrouched + CrouchSpeed, InitialCrouchSpeed, MaxCrouchSpeed);
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr)	return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastBuffSpeed(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr)	return;
	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = FMath::Clamp(Character->GetCharacterMovement()->JumpZVelocity + BuffJumpVelocity, InitialJumpVelocity, MaxJumpVelocity);
	}
	MulticastJumpVelocity(BuffJumpVelocity);
}

void UBuffComponent::ResetJump()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr)	return;
	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastJumpVelocity(InitialJumpVelocity);
}

void UBuffComponent::MulticastJumpVelocity_Implementation(float JumpVelocity)
{
	Character->GetCharacterMovement()->JumpZVelocity = FMath::Clamp(Character->GetCharacterMovement()->JumpZVelocity + JumpVelocity, InitialJumpVelocity, MaxJumpVelocity);
}
