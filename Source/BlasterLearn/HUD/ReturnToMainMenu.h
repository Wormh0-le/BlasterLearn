// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERLEARN_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	void MenuSetup();
	void MenuTeardown();

protected:
	virtual bool Initialize() override;

	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* ReturnButton;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	class UXMultiPlayerSessionsSubsystem* XMultiPlayerSessionsSubsystem;

	UPROPERTY()
	class APlayerController* PlayerController;
};
