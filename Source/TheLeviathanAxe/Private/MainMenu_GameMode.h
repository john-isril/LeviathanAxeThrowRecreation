// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenu_GameMode.generated.h"

class UMainMenu;

/**
 * 
 */
UCLASS()
class AMainMenu_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AMainMenu_GameMode();

private:

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (DisplayName = "Main Menu Class"))
	TSubclassOf<UUserWidget> m_MainMenuClass;

	UPROPERTY(EditAnywhere, Category = "Game", meta = (DisplayName = "Main Level"))
	FName m_MainLvl{"Main_Level"};

	UPROPERTY(Transient)
	TObjectPtr<UMainMenu> m_ControlsMenu{ nullptr };

private:
	
	UFUNCTION()
	void OnPlayBtnClicked();

protected:
	virtual void BeginPlay() override;
};
