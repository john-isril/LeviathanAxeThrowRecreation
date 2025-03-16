// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ControlsMenu.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class UI_API UControlsMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackBtn{ nullptr };
	
};
