// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Main_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class AMain_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	void OnPawnKilled(APawn* KilledPawn);
};
