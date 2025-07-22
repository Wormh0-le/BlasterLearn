// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterLearn/BlasterTypes/Team.h"
#include "GameFrameWork/PlayerStart.h"
#include "TeamPlayerStart.generated.h"

UCLASS()
class BLASTERLEARN_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ETeam Team;
};
