// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	if (ProjectileClass == nullptr || NonReplicatedProjectileClass == nullptr) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World) {
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from traceundercrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;
		if (InstigatorPawn) {
			if (InstigatorPawn->HasAuthority()) // server, no SSR
			{
				if (!InstigatorPawn->IsLocallyControlled() && bUseServerSideRewind)	// not locally controlled, no replicated
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						NonReplicatedProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);
				}
				else
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(	// locally controlled or bUseServerSideRewind, just replicated
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);	
				}
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
			else // client
			{
				if (bUseServerSideRewind)
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(
						NonReplicatedProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);
					SpawnedProjectile->bUseServerSideRewind = false;	// not locally controlled, no SSR
					SpawnedProjectile->Damage = Damage;
					if (InstigatorPawn->IsLocallyControlled())
					{
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					}
				}
			}
		}
	}
}
