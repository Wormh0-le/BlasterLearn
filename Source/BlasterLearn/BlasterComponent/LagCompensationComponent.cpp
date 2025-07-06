// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "BlasterLearn/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "BlasterLearn/BlasterLearn.h"
#include "BlasterLearn/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFrameHistory();
}

void ULagCompensationComponent::UpdateFrameHistory()
{
	if (Character == nullptr || !Character->HasAuthority())	return;
	FFramePackage CurFrame;
	SaveFramePackage(CurFrame);
	if (FrameHistory.Num() <= 1)
	{
		FrameHistory.AddHead(CurFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FrameHistory.AddHead(CurFrame);
		// ShowFramePackage(CurFrame, FColor::Green);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
		}
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (Package.Character == nullptr)	return FServerSideRewindResult();

	FFramePackage CurFrame;
	CacheBoxPositions(Package.Character, CurFrame);
	MoveBoxes(Package.Character, Package);
	EnableCharacterMeshCollision(Package.Character, ECollisionEnabled::NoCollision);

	// Enable collision for the head first
	UBoxComponent* HeadBox = Package.Character->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);	
		if (ConfirmHitResult.bBlockingHit)
		{
			if (ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
			}
			ResetBoxes(Package.Character, CurFrame);
			EnableCharacterMeshCollision(Package.Character, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else
		{
			for (auto& HitBoxPair : Package.Character->HitCollisionBoxes)
			{
				if (HitBoxPair.Key == FName("head"))
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					continue;
				} 
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
				}
				World->LineTraceSingleByChannel(
					ConfirmHitResult,
					TraceStart,
					TraceEnd,
					ECC_HitBox
				);
				if (ConfirmHitResult.bBlockingHit)
				{
					if (ConfirmHitResult.Component.IsValid())
					{
						UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
						if (Box)
						{
							DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
						}
					}
					ResetBoxes(Package.Character, CurFrame);
					EnableCharacterMeshCollision(Package.Character, ECollisionEnabled::QueryAndPhysics);
					return FServerSideRewindResult{ true, false };
				}
			}
		}
	}
	ResetBoxes(Package.Character, CurFrame);
	EnableCharacterMeshCollision(Package.Character, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

FBatchServerSideRewindResult ULagCompensationComponent::BatchServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
		if (FrameToCheck.Character == nullptr)
		{
			return FBatchServerSideRewindResult();
		}
		FramesToCheck.Add(FrameToCheck);
	}

	return BatchConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FBatchServerSideRewindResult ULagCompensationComponent::BatchConfirmHit(const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	TArray<FFramePackage> CurrentFrames;
	FBatchServerSideRewindResult BatchConfirmHitResult;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurFrame;
		CurFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurFrame);
		MoveBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurFrame);

		// Enable collision for the head first
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
	}
	FHitResult ConfirmHitResult;
	UWorld* World = GetWorld();
	if (World == nullptr)	return BatchConfirmHitResult;
	// check for headshots
	for (auto& HitLocation : HitLocations)
	{
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (ConfirmHitResult.bBlockingHit)
		{
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (HitCharacter)
			{
				if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
					}
				}
				if (BatchConfirmHitResult.HeadShots.Contains(HitCharacter))
				{
					BatchConfirmHitResult.HeadShots[HitCharacter]++;
				} else
				{
					BatchConfirmHitResult.HeadShots.Emplace(HitCharacter, 1);
				}
			}
		}
	}

	// check for body shots
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Key == FName("head"))
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue; // skip head box
			}
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
		}
	}
	for (auto& HitLocation : HitLocations)
	{
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (ConfirmHitResult.bBlockingHit)
		{
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (HitCharacter)
			{
				if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					}
				}
				if (BatchConfirmHitResult.BodyShots.Contains(HitCharacter))
				{
					BatchConfirmHitResult.BodyShots[HitCharacter]++;
				} else
				{
					BatchConfirmHitResult.BodyShots.Emplace(HitCharacter, 1);
				}
			}
		}
	}
	// Reset boxes and character mesh collision
	for (auto& CurFrame : CurrentFrames)
	{
		ResetBoxes(CurFrame.Character, CurFrame);
		EnableCharacterMeshCollision(CurFrame.Character, ECollisionEnabled::QueryAndPhysics);
	}
 	return BatchConfirmHitResult;
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
bool bReturn =
		HitCharacter == nullptr || HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;
	if (bReturn)	return FFramePackage();

	// FramePackage that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	// frame history of the hit character
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime)
	{
		// too far back in time, so we can't rewind. too laggy do SSR
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}
	auto RightFramePackage = History.GetHead();
	auto LeftFramePackage = RightFramePackage;
	while (LeftFramePackage->GetValue().Time > HitTime)
	{
		if (LeftFramePackage->GetNextNode() == nullptr)	break;
		LeftFramePackage = LeftFramePackage->GetNextNode();
		if (LeftFramePackage->GetValue().Time > HitTime)
		{
			RightFramePackage = LeftFramePackage;
		}
	}
	if (LeftFramePackage->GetValue().Time == HitTime)
	{
		FrameToCheck = LeftFramePackage->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		// Interpolate between the two frames
		FrameToCheck = InterpBetweenFrames(LeftFramePackage->GetValue(), RightFramePackage->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	FrameToCheck.Time = HitTime;
	return FrameToCheck;
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& LeftFrame,
	const FFramePackage& RightFrame, float HitTime)
{
	const float Distance = RightFrame.Time - LeftFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - LeftFrame.Time) / Distance, 0.f, 1.f);
	FFramePackage InterpFrame;
	for (auto& Pair : LeftFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = Pair.Key;
		const FBoxInformation& LeftBoxInfo = Pair.Value;
		const FBoxInformation& RightBoxInfo = RightFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(LeftBoxInfo.Location, RightBoxInfo.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(LeftBoxInfo.Rotation, RightBoxInfo.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = LeftBoxInfo.BoxExtent;

		InterpFrame.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	return InterpFrame;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime,
	AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	if (Character && DamageCauser && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Confirm.bHeadShot ? DamageCauser->GetHeadShotDamage() : DamageCauser->GetDamage(),
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::BatchServerScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime,
	class AWeapon* DamageCauser)
{
	FBatchServerSideRewindResult BatchConfirm = BatchServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	if (DamageCauser)
	{
		for (auto& HitCharacter : HitCharacters)
		{
			if (HitCharacter == nullptr || Character == nullptr)	continue;
			float TotalDamage = 0.f;
			if (BatchConfirm.HeadShots.Contains(HitCharacter))
			{
				TotalDamage += BatchConfirm.HeadShots[HitCharacter] * DamageCauser->GetHeadShotDamage();
			}
			if (BatchConfirm.BodyShots.Contains(HitCharacter))
			{
				TotalDamage += BatchConfirm.BodyShots[HitCharacter] * DamageCauser->GetDamage();
			}
			UGameplayStatics::ApplyDamage(
				HitCharacter,
				TotalDamage,
				Character->Controller,
				DamageCauser,
				UDamageType::StaticClass()
			);
		}
	}
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr)	return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr)	return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr)	return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, FColor Color)
{
	for (auto& BoxInfo :Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

