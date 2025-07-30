// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterLearn/BlasterTypes/KillEventInfo.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "BannerWidget.generated.h"


USTRUCT(BlueprintType)
struct FKillBannerStyleRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EKillEventType KillType = EKillEventType::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BackgroundTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* KillIcon{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* BackgroundMaterial{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConsecutiveKillState = false ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* VoiceCue{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDuration = .1f;
};


/**
 * 
 */
UCLASS()
class BLASTERLEARN_API UBannerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateBanner(const FKillEventMessage& KillEventMessage);

	TQueue<FKillEventMessage> KillEventQueue = TQueue<FKillEventMessage>();
	FThreadSafeCounter KillEventCount = FThreadSafeCounter(0);
protected:
	UPROPERTY(meta = (BindWidget))
	class UBorder* Background;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SubjectText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* VerbIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ObjectText;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* BannerStyleDataTable;

	FKillBannerStyleRow* BannerStyle;

private:
	float Duration = 0.f;

public:
	FORCEINLINE float GetDisplayDuration() const { return Duration; }
};
