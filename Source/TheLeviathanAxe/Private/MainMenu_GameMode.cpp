// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu_GameMode.h"
#include "MainMenu.h"
#include <Components/Button.h>
#include <Kismet/GameplayStatics.h>

AMainMenu_GameMode::AMainMenu_GameMode()
{
	DefaultPawnClass = nullptr;
}

void AMainMenu_GameMode::BeginPlay()
{
	APlayerController* const PlayerController{ GetWorld()->GetFirstPlayerController() };

	if (PlayerController)
	{
		PlayerController->SetPause(true);
		PlayerController->SetInputMode(FInputModeUIOnly{});
		PlayerController->bShowMouseCursor = true;

		if (ensureAlwaysMsgf(m_MainMenuClass, TEXT("Main Menu Class is not set!")))
		{
			m_ControlsMenu = CreateWidget<UMainMenu>(PlayerController, m_MainMenuClass);
			
			if (m_ControlsMenu)
			{
				m_ControlsMenu->AddToViewport();
				m_ControlsMenu->PlayBtn->OnClicked.AddUniqueDynamic(this, &AMainMenu_GameMode::OnPlayBtnClicked);
			}
		}
	}
}

void AMainMenu_GameMode::OnPlayBtnClicked()
{
	if (m_ControlsMenu)
	{
		m_ControlsMenu->RemoveFromParent();
		m_ControlsMenu = nullptr;
	}

	if (APlayerController* const PlayerController{ GetWorld()->GetFirstPlayerController() })
	{
		PlayerController->SetPause(false);
		PlayerController->SetInputMode(FInputModeGameOnly{});
		PlayerController->bShowMouseCursor = false;
	}

	UGameplayStatics::OpenLevel(this, m_MainLvl);
}