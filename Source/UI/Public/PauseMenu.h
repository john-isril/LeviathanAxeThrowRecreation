// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class UI_API UPauseMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResumeBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ControlsBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CreditsBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RestartBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitBtn;
};
