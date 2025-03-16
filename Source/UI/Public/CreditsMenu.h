// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreditsMenu.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class UI_API UCreditsMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackBtn{ nullptr };
};
