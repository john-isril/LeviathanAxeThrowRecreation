// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroOverlay.h"
#include <Components/Image.h>
#include "Blueprint/SlateBlueprintLibrary.h"
#include <Components/ProgressBar.h>
#include "HealthComponent.h"

void UHeroOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	DisplayCrosshairs(false);

	UHealthComponent* const OwningPawnsHealthComponent{ GetOwningPlayerPawn()->GetComponentByClass<UHealthComponent>() };

	if (ensureAlwaysMsgf(CrosshairsTexture, TEXT("The owning player pawn does not have a health component (UHealthComponent) !")))
	{
		OwningPawnsHealthComponent->OnHealthChange.BindUObject(this, &UHeroOverlay::OnHealthChanged);
	}
}

void UHeroOverlay::DisplayCrosshairs(bool bDisplay)
{
	if (ensureAlwaysMsgf(CrosshairsTexture, TEXT("Crosshairs Texture not set!")))
	{
		CrosshairsTexture->SetVisibility(bDisplay ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

FVector2D UHeroOverlay::GetCrosshairsScreenPosition() const
{
	if (!ensureAlwaysMsgf(CrosshairsTexture, TEXT("Crosshairs Texture not set!"))) return {};

	FVector2D PixelPosition{}; // Unreal describes the position we need as the "PixelPosition"
	FVector2D ViewportPosition{}; // We aren't going to use this variable, but we need something to pass into the next functions "ViewportPosition" parameter, which is something different from what we need even though the name implies it's the position in the viewport.
	const FVector2D CrosshairsTextureHalfSize{ CrosshairsTexture->GetDesiredSize() * 0.5f }; // This is the local offset we need to get to the center of our crosshairs texture. The origin (0, 0) will be the top left of the crosshairs texture, which we don't want.

	USlateBlueprintLibrary::LocalToViewport(CrosshairsTexture, CrosshairsTexture->GetCachedGeometry(), CrosshairsTextureHalfSize, PixelPosition, ViewportPosition);
	return PixelPosition;
}

void UHeroOverlay::OnHealthChanged(float CurrentHealth, float MaxHealth)
{
	checkf(!FMath::IsNearlyZero(MaxHealth), TEXT("MaxHealth is zero! Divide by zero error!"));
	HealthBar->SetPercent(CurrentHealth / MaxHealth);
}
