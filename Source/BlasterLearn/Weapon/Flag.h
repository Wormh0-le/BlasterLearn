// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();
	virtual void Dropped() override;

	void SetTeamColor();
protected:
	virtual void BeginPlay() override;
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
	virtual void OnRep_Owner() override;

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* FlagMesh;
	
	UPROPERTY(VisibleAnywhere, Category = Team)
	UMaterialInstanceDynamic* DynamicTeamMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Team)
	UMaterialInstance* TeamMaterialInstance;
};
