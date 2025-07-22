// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "BlasterLearn/BlasterTypes/Team.h"
#include "BlasterLearn/Character/BlasterCharacter.h"
#include "Components/WidgetComponent.h"


AFlag::AFlag()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	if (TeamMaterialInstance)
	{
		DynamicTeamMaterialInstance = UMaterialInstanceDynamic::Create(TeamMaterialInstance, this);
		FlagMesh->SetMaterial(0, DynamicTeamMaterialInstance);
		DynamicTeamMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor::Yellow);
	}
}


void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
	SetTeamColor();
}

void AFlag::SetTeamColor()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (DynamicTeamMaterialInstance)
	{
		if (BlasterOwnerCharacter)
		{
			BlasterOwnerCharacter->AttachFlagToLeftHand(this);
			switch (BlasterOwnerCharacter->GetTeam())
			{
			case ETeam::ET_BlueTeam:
				DynamicTeamMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor::Blue);
				break;
			case ETeam::ET_RedTeam:
				DynamicTeamMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor::Red);
				break;
			default:
				DynamicTeamMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor::Yellow);
				break;
			}	
		}
		else
		{
			DynamicTeamMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor::Yellow);
		}
	}
}



void AFlag::OnEquipped()
{
	ShowPickUpWidget(false);
	// server side
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);
}

void AFlag::OnDropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);	
	}
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void AFlag::OnRep_Owner()
{
	Super::OnRep_Owner();
	SetTeamColor();
}
