// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_GameMode.h"
#include <AIController.h>
#include <BrainComponent.h>
#include "Engine.h"

void AMain_GameMode::OnPawnKilled(APawn* KilledPawn)
{
	if (KilledPawn->GetController() == GetWorld()->GetFirstPlayerController())
	{
		/* Disable all AI logic */
		for (FConstControllerIterator ControllerIterator{ GetWorld()->GetControllerIterator() }; ControllerIterator; ++ControllerIterator)
		{
			if (AAIController* const AIController{ Cast<AAIController>(ControllerIterator->Get()) })
			{
				AIController->GetBrainComponent()->StopLogic(FString{ "Won" });
			}
		}
	}

}
