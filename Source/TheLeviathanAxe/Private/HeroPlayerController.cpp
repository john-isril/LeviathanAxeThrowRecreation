// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroPlayerController.h"
#include "PauseMenu.h"
#include "ControlsMenu.h"
#include "CreditsMenu.h"
#include <Components/Button.h>
#include <Kismet/KismetSystemLibrary.h>

void AHeroPlayerController::OpenPauseMenu(bool bOpen)
{
	if (bOpen)
	{
		SetPause(true);
		SetInputMode(FInputModeGameAndUI{});
		bShowMouseCursor = true;

		if (!ensureAlwaysMsgf(m_PauseMenuWidgetClass, TEXT("Pause Menu Widget Class is not set!"))) return;
		
		m_PauseMenu = CreateWidget<UPauseMenu>(this, m_PauseMenuWidgetClass);
		
		if (!m_PauseMenu.IsValid()) return;
		
		m_PauseMenu->AddToViewport();
		m_PauseMenu->ResumeBtn->OnClicked.AddUniqueDynamic(this, &AHeroPlayerController::TogglePauseMenu);
		m_PauseMenu->ControlsBtn->OnClicked.AddUniqueDynamic(this, &AHeroPlayerController::OnControlsBtnClicked);
		m_PauseMenu->RestartBtn->OnClicked.AddUniqueDynamic(this, &APlayerController::RestartLevel);
		m_PauseMenu->CreditsBtn->OnClicked.AddUniqueDynamic(this, &AHeroPlayerController::OnCreditsBtnClicked);
		m_PauseMenu->QuitBtn->OnClicked.AddUniqueDynamic(this, &AHeroPlayerController::OnQuitBtnClicked);
	}
	else
	{
		SetPause(false);
		SetInputMode(FInputModeGameOnly{});
		bShowMouseCursor = false;

		if (!m_PauseMenu.IsValid()) return;
		
		m_PauseMenu->RemoveFromParent();
		m_PauseMenu.Reset();
	}
}

void AHeroPlayerController::TogglePauseMenu()
{
	OpenPauseMenu(!IsPaused());
}

void AHeroPlayerController::RestartLevel()
{
	if (IsPaused())
	{
		OpenPauseMenu(false);
	}
	
	Super::RestartLevel();
}

void AHeroPlayerController::OnQuitBtnClicked()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, true);
}

void AHeroPlayerController::OnControlsBtnClicked()
{
	if (m_PauseMenu.IsValid())
	{
		m_PauseMenu->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!ensureAlwaysMsgf(m_ControlsMenuWidgetClass, TEXT("Controls Menu Widget Class is not set!"))) return;
	
	m_ControlsMenu = CreateWidget<UControlsMenu>(this, m_ControlsMenuWidgetClass);
	
	if (!m_ControlsMenu.IsValid()) return;
	
	m_ControlsMenu->AddToViewport();
	m_ControlsMenu->BackBtn->OnClicked.AddUniqueDynamic(this, &AHeroPlayerController::OnControlsMenuBackBtnClicked);
}

void AHeroPlayerController::OnControlsMenuBackBtnClicked()
{
	if (ensureAlwaysMsgf(m_ControlsMenu.IsValid(), TEXT("m_ControlsMenu is null!")))
	{
		m_ControlsMenu->RemoveFromParent();
		m_ControlsMenu = nullptr;
	}

	if (!ensureAlwaysMsgf(m_PauseMenu.IsValid(), TEXT("m_PauseMenu is null!"))) return;
	m_PauseMenu->SetVisibility(ESlateVisibility::Visible);
}

void AHeroPlayerController::OnCreditsBtnClicked()
{
	if (m_PauseMenu.IsValid())
	{
		m_PauseMenu->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!ensureAlwaysMsgf(m_ControlsMenuWidgetClass, TEXT("Credits Menu Widget Class is not set!"))) return;

	m_CreditsMenu = CreateWidget<UCreditsMenu>(this, m_CreditsMenuWidgetClass);

	if (!m_CreditsMenu.IsValid()) return;

	m_CreditsMenu->AddToViewport();
	m_CreditsMenu->BackBtn->OnClicked.AddUniqueDynamic(this, &AHeroPlayerController::OnCreditsMenuBackBtnClicked);
}

void AHeroPlayerController::OnCreditsMenuBackBtnClicked()
{
	if (ensureAlwaysMsgf(m_CreditsMenu.IsValid(), TEXT("m_CreditsMenu is null!")))
	{
		m_CreditsMenu->RemoveFromParent();
		m_CreditsMenu = nullptr;
	}

	if (!ensureAlwaysMsgf(m_PauseMenu.IsValid(), TEXT("m_PauseMenu is null!"))) return;
	m_PauseMenu->SetVisibility(ESlateVisibility::Visible);
}