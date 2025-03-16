// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthBarWidget.h"
#include <Components/ProgressBar.h>

void UEnemyHealthBarWidget::SetPercent(float Percent)
{
	if (ensureAlwaysMsgf(HealthBar, TEXT("Health Bar is null!")))
	{
		HealthBar->SetPercent(Percent);
	}

	/* Once this timer is complete, the DelayedHealthBar should start interpolating it's percent to the actual healh percent. */
	GetWorld()->GetTimerManager().SetTimer(m_HealthBarDecreaseDelay, [this]() 
	{
		m_bIsDecreasingFill = true;
	}, s_HealthBarDecreaseDelayDuration, false);
}

void UEnemyHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	m_bIsDecreasingFill = false;
}

void UEnemyHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	/* Interpolate the DelayedHealthBar's percent to the actual health percentage. */
	if (m_bIsDecreasingFill)
	{
		const float TargetPercent{ HealthBar->GetPercent() };
		float CurrentPercent{ DelayedHealthBar->GetPercent() };
		CurrentPercent = FMath::FInterpTo(CurrentPercent, TargetPercent, InDeltaTime, s_HealthBarInterpSpeed);
		
		if (ensureAlwaysMsgf(HealthBar, TEXT("Delayed Health Bar is null!")))
		{
			DelayedHealthBar->SetPercent(CurrentPercent);
		}

		if (FMath::IsNearlyEqual(CurrentPercent, TargetPercent))
		{
			m_bIsDecreasingFill = false;
		}
	}
}
