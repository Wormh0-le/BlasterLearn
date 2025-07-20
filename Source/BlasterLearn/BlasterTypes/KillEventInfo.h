#pragma once
#include "KillEventInfo.generated.h"


UENUM(BlueprintType)
enum class EKillEventType : uint8
{
	Kill     UMETA(DisplayName = "Kill"),
	Headshot       UMETA(DisplayName = "Headshot"),
	Shutdown       UMETA(DisplayName = "Shutdown"),
	DoubleKill     UMETA(DisplayName = "DoubleKill"),
	TripleKill     UMETA(DisplayName = "TripleKill"),
	QuadraKill     UMETA(DisplayName = "QuadraKill"),
	PentaKill      UMETA(DisplayName = "PentaKill"),
	TeamKill       UMETA(DisplayName = "TeamKill"),
	KillingSpree   UMETA(DisplayName = "KillingSpree"),
	Rampage   UMETA(DisplayName = "Rampage"),
	Unstoppable    UMETA(DisplayName = "Unstoppable"),
	Dominating    UMETA(DisplayName = "Dominating"),
	Godlike        UMETA(DisplayName = "Godlike"),
	Legendary      UMETA(DisplayName = "Legendary"),
	Default		UMETA(DisplayName = "Default")
};

FORCEINLINE FText GetKillTypeDisplayName(EKillEventType KillType)
{
	const UEnum* EnumPtr = StaticEnum<EKillEventType>();
	if (!EnumPtr) return FText::FromString("Invalid");

	return EnumPtr->GetDisplayNameTextByValue((int64)KillType);
}



USTRUCT(BlueprintType)
struct FKillEventMessage
{
	GENERATED_BODY()

	UPROPERTY()
	FString KillerName;

	UPROPERTY()
	FString VictimName;

	UPROPERTY()
	EKillEventType KillType = EKillEventType::Default;

	FKillEventMessage() {};
	
	FKillEventMessage(const FString& InKillerName, const FString& InVictimName, EKillEventType InKillType)
		: KillerName(InKillerName), VictimName(InVictimName), KillType(InKillType) {}
};