// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HeroHUD.generated.h"

class UHeroOverlay;

/**
 * 
 */
UCLASS()
class UI_API AHeroHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	
	void DisplayCrosshairs(bool bDisplay = true);
	
	FVector2D GetCrosshairsScreenPosition() const;

private:
	
	UPROPERTY(EditAnywhere, Category = "UI", meta = (DisplayName = "Hero Overlay Class"))
	TSubclassOf<UUserWidget> m_HeroOverlayClass{};

	UPROPERTY()
	TObjectPtr<UHeroOverlay> m_HeroOverlay{ nullptr };

private:

	virtual void BeginPlay() override;

};
