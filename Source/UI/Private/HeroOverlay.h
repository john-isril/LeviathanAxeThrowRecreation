// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroOverlay.generated.h"

class UImage;
class UProgressBar;

/**
 * 
 */
UCLASS()
class UHeroOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void DisplayCrosshairs(bool bDisplay = true);
	
	FVector2D GetCrosshairsScreenPosition() const;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CrosshairsTexture{ nullptr };
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar{ nullptr };

private:
	virtual void NativeConstruct() override;
	void OnHealthChanged(float CurrentHealth, float MaxHealth);
};
