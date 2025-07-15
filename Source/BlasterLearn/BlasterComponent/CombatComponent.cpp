// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "BlasterLearn/Weapon/Weapon.h"
#include "BlasterLearn/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "BlasterLearn/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "BlasterLearn/Weapon/Projectile.h"
#include "BlasterLearn/Weapon/Shotgun.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 400.f;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		InitializeCarriedAmmo();
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled()) {
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld) {
		FVector Start = CrosshairWorldPosition;
		if (Character) {
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 50.f);
			//DrawDebugSphere(GetWorld(), Start, 5.f, 12, FColor::Red, false);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit){
			TraceHitResult.ImpactPoint = End;
		}
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()) {
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		} else {
			HUDPackage.CrosshairsColor = FLinearColor::Green;
		}
	}

}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr)	return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller) {
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;

		if (HUD) {
			if (EquippedWeapon) {
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				
			} else {
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			// Calculate crosshair spread
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			if (bAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			} else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
			
			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)	return;
	if (bAiming) {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	} else {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera()) {
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)	return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr)	return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	ReloadEmptyWeapon();
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)	return false;
	// Shotgun can interupt reload
	bool bShotgunReloading = CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun;
	if (bShotgunReloading)	return true;
	if (bLocallyReloading)	return false;
	if (Character && !Character->bFinishSwapping)	return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

// void UCombatComponent::OnRep_CarriedAmmo()
// {
// 	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
// 	if (Controller)
// 	{
// 		Controller->SetHUDCarriedAmmo(CarriedAmmo);
// 	}
// 	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
// 		EquippedWeapon && 
// 		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun && 
// 		CarriedAmmo == 0;
// 	if (bJumpToShotgunEnd) {
// 		JumpToShotgunEnd();
// 	}
// }

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		if (EquippedWeapon) {
			bCanFire = false;
			CrosshairShootingFactor = 0.75f;
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Projectile:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			}		
		}
		StartFireTimer();	
	}
	
}

void UCombatComponent::FireProjectileWeapon()
{
	if (Character)
	{
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget);	
	}
	
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		FireProjectileWeapon();
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> ScatteredHitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, ScatteredHitTargets);
		if (!Character->HasAuthority()) LocalShotgunFire(ScatteredHitTargets);
		ServerShotgunFire(ScatteredHitTargets);
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr)	return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& ScatteredHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr)	return;
	
	bool bShotgunReloading = CombatState == ECombatState::ECS_Reloading;
	if (Character && (CombatState == ECombatState::ECS_Unoccupied || bShotgunReloading))
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(ScatteredHitTargets);
		if (bShotgunReloading) {
			CombatState = ECombatState::ECS_Unoccupied;
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& ScatteredHitTargets)
{
	MulticastShotgunFire(ScatteredHitTargets);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())	return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& ScatteredHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())	return;
	LocalShotgunFire(ScatteredHitTargets);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr)	return;
	bAiming = bIsAiming;
	// client owned actor call on server to replicate bIsAiming
	ServerSetAiming(bIsAiming);
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled())
	{
		bAimingButtonPressed = bIsAiming;
		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) Character->ShowSniperScopeWidget(bIsAiming);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	// server should also know my speed
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_Aiming()
{
	// replicate is more efficient than server rpc, 
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimingButtonPressed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character) {
		// make sure weapon state property set first before attach actor gets called
		// because attach Actor is also replicated 
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo();
		UpdateCarriedAmmo();
	} else
	{
		AttachActorToRightHand((AActor*)nullptr);
		ResetCombat();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character) {
		// make sure weapon state property set first before attach actor gets called
		// because attach Actor is also replicated 
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(SecondaryWeapon);
	} else
	{
		AttachActorToBackpack((AActor*)nullptr);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied)	return;
	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	} else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	
	// Offset Yaw for Strafing
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::ResetCombat()
{
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	Character->bUseControllerRotationYaw = false;
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(0);
		Controller->SetHUDWeaponAmmo(0);
	}
}

void UCombatComponent::ThrowEquippedWeapon()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied)	return;
	DropEquippedWeapon();
	// client, local prediction
	if (SecondaryWeapon) {
		EquipPrimaryWeapon(SecondaryWeapon);
		SecondaryWeapon = nullptr;
	} else
	{
		ResetCombat();	
	}
	if (Character && !Character->HasAuthority())
	{
		ServerOnThrowEquippedWeapon();
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	// owner has been replicated
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);
	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetOwner(Character);
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	
	PlayEquipWeaponSound(SecondaryWeapon);
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
		FVector ThrowDirection = (HitTarget - Character->GetActorLocation()).GetSafeNormal();
		EquippedWeapon->GetWeaponMesh()->AddImpulse(ThrowDirection * ThrowStrength, NAME_None, true);
		EquippedWeapon = nullptr;
	}
	if (Character && !Character->HasAuthority())
	{
		ServerDropEquippedWeapon(HitTarget);
	}
}

void UCombatComponent::ServerDropEquippedWeapon_Implementation(const FVector_NetQuantize& Target)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
		FVector ThrowDirection = (Target - Character->GetActorLocation()).GetSafeNormal();
		EquippedWeapon->GetWeaponMesh()->AddImpulse(ThrowDirection * ThrowStrength, NAME_None, true);
		EquippedWeapon = nullptr;	
	}
}

void UCombatComponent::ServerOnThrowEquippedWeapon_Implementation()
{
	if (SecondaryWeapon) {
		EquipPrimaryWeapon(SecondaryWeapon);
		SecondaryWeapon = nullptr;
	} else
	{
		ResetCombat();	
	}
}


void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket) {
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;
	EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
	bool bUsePistolSocket = 
		WeaponType == EWeaponType::EWT_Pistol || 
		WeaponType == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket) {
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr)	return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
	}
	if (EquippedWeapon && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		UpdateCarriedAmmo();
		if (EquippedWeapon->IsEmpty()) Reload();
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState) {
	case ECombatState::ECS_Reloading:
		if (Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) {
			Fire();
		}	
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	}
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		ServerReload();
		if (!Character->HasAuthority())	HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr)	return;

	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled())	HandleReload();
}

void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
		if (Character->HasAuthority()) {
			CombatState = ECombatState::ECS_Unoccupied;
		}
	}
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bLocallyReloading = false;
	
	UpdateAmmoValues();
	if (bFireButtonPressed) {
		Fire();
	}
}

void UCombatComponent::SwapWeapons()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr)	return;

	Character->PlaySwapMontage();
	Character->bFinishSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapons;
	// descrease afterimage
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if (Character)	Character->bFinishSwapping = true;
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}

void UCombatComponent::SwapAttachedWeapon()
{
	if (Character == nullptr || !Character->HasAuthority())	return;
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);
	ReloadEmptyWeapon();

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr)	return;
	EWeaponType EquippedWeaponType = EquippedWeapon->GetWeaponType();
	bool bShotgun = EquippedWeaponType == EWeaponType::EWT_ShotGun;
	int32 ReloadAmount = bShotgun ? 1 : AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeaponType)) {
		CarriedAmmoMap[EquippedWeaponType] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeaponType];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(ReloadAmount);
	if (bShotgun)
	{
		bCanFire = true;
		if (EquippedWeapon->IsFull() || CarriedAmmo == 0) {
			JumpToShotgunEnd();
		}	
	}
	if (Character->HasAuthority())
	{
		ClientUpdateAmmoValues(CarriedAmmo, ReloadAmount);
	}
	else
	{
		ReloadSequence += ReloadAmount;
	}
}

void UCombatComponent::ClientUpdateAmmoValues_Implementation(int32 ServerCarriedAmmo, int32 ServerReloadAmount)
{
	if (Character == nullptr || Character->HasAuthority() || EquippedWeapon == nullptr)	return;
	EWeaponType EquippedWeaponType = EquippedWeapon->GetWeaponType();
	bool bShotgun = EquippedWeaponType == EWeaponType::EWT_ShotGun;
	int32 ReloadAmount = ServerReloadAmount;
	CarriedAmmo = ServerCarriedAmmo;
	ReloadSequence -= ServerReloadAmount;
	ReloadAmount -= ReloadSequence;
	CarriedAmmo -= ReloadSequence;
	if (CarriedAmmoMap.Contains(EquippedWeaponType)) {
		CarriedAmmoMap[EquippedWeaponType] = CarriedAmmo;
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(ReloadAmount);
	if (bShotgun)
	{
		bCanFire = true;
		if (EquippedWeapon->IsFull() || CarriedAmmo == 0) {
			JumpToShotgunEnd();
		}	
	}
}

// void UCombatComponent::UpdateShotgunAmmoValues()
// {
// 	if (Character == nullptr || EquippedWeapon == nullptr)	return;
// 	// int32 ReloadAmount = AmountToReload();
// 	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
// 		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
// 		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
// 	}
// 	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
// 	if (Controller)
// 	{
// 		Controller->SetHUDCarriedAmmo(CarriedAmmo);
// 	}
// 	EquippedWeapon->AddAmmo(1);
// 	bCanFire = true;
// 	if (EquippedWeapon->IsFull() || CarriedAmmo == 0) {
// 		JumpToShotgunEnd();
// 	}
// }

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}


void UCombatComponent::ShotgunShellReload()
{
	if(Character && Character->HasAuthority() && !EquippedWeapon->IsFull()) {
		UpdateAmmoValues();
	}
	
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade()) {
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr)	return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority()) {
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Character;
		SpawnParameters.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParameters
			);
		}
	} 
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::InitialGrenades()
{
	Grenades = MaxGrenades;
	UpdateHUDGrenades();
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

bool UCombatComponent::CanSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	// DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
}

