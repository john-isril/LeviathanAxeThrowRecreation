// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroHUD.h"
#include "HealthComponent.h"
#include "HeroOverlay.h"

void AHeroHUD::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureAlwaysMsgf(m_HeroOverlayClass, TEXT("Hero Overlay Class not set!"))) return;

	m_HeroOverlay = CreateWidget<UHeroOverlay>(GetOwningPlayerController(), m_HeroOverlayClass);

	if (m_HeroOverlay)
	{
		m_HeroOverlay->AddToViewport();
	}
}

void AHeroHUD::DisplayCrosshairs(bool bDisplay)
{
	if (m_HeroOverlay)
	{
		m_HeroOverlay->DisplayCrosshairs(bDisplay);
	}
}

FVector2D AHeroHUD::GetCrosshairsScreenPosition() const
{
	if (!ensureAlwaysMsgf(m_HeroOverlay, TEXT("m_HeroOverlay is null!"))) return {};
	
	return m_HeroOverlay->GetCrosshairsScreenPosition();
}
