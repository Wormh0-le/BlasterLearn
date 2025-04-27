// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	
	FString NetRoleString = FString::Printf(TEXT("Local role: %s \n Remote role: %s"), *GetNetRole(LocalRole), *GetNetRole(RemoteRole));
	SetDisplayText(NetRoleString);
}

FString UOverheadWidget::GetNetRole(ENetRole NetRole)
{
	FString Role;
	switch (NetRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}
	return Role;
}

void UOverheadWidget::NativeDestruct()
{
	Super::NativeDestruct();
}
