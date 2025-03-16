// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

DECLARE_DELEGATE(FOnPlayBtnClickedSignature);

class UButton;

/**
 * 
 */
UCLASS()
class UI_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayBtn{ nullptr };
};
