// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HeroPlayerController.generated.h"

class UPauseMenu;
class UControlsMenu;
class UCreditsMenu;

/**
 * 
 */
UCLASS()
class AHeroPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void OpenPauseMenu(bool bOpen = true);

	UFUNCTION()
	void TogglePauseMenu();

	virtual void RestartLevel() override;

private:
	
	UPROPERTY(EditAnywhere, Category = "UI", meta = (DisplayName = "Pause Menu Widget Class"))
	TSubclassOf<UUserWidget> m_PauseMenuWidgetClass{};

	UPROPERTY(Transient)
	TWeakObjectPtr<UPauseMenu> m_PauseMenu{};

	UPROPERTY(EditAnywhere, Category = "UI", meta = (DisplayName = "Controls Menu Widget Class"))
	TSubclassOf<UUserWidget> m_ControlsMenuWidgetClass{};

	UPROPERTY(Transient)
	TWeakObjectPtr<UControlsMenu> m_ControlsMenu{};

	UPROPERTY(EditAnywhere, Category = "UI", meta = (DisplayName = "Credits Menu Widget Class"))
	TSubclassOf<UUserWidget> m_CreditsMenuWidgetClass{};

	UPROPERTY(Transient)
	TWeakObjectPtr<UCreditsMenu> m_CreditsMenu{};

private:

	/*Open controls menu.*/
	UFUNCTION()
	void OnControlsBtnClicked();

	/*Return back to the main pause menu from the controls menu.*/
	UFUNCTION()
	void OnControlsMenuBackBtnClicked();

	UFUNCTION()
	void OnCreditsBtnClicked();

	/*Return back to the main pause menu from the credits menu.*/
	UFUNCTION()
	void OnCreditsMenuBackBtnClicked();

	UFUNCTION()
	void OnQuitBtnClicked();
	
};
