// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class UProgressBar;

/**
 * 
 */
UCLASS()
class AI_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetPercent(float Percent);

private:
	/* Progress bar accurately representing the current health. */
	UPROPERTY(VisibleAnywhere, meta = (BindWidget, DisplayName = "Health Bar"))
	TObjectPtr<UProgressBar> HealthBar{ nullptr };

	/*Progress bar solely meant for animating the reduction of health. This is purposely delayed and does not always accurately represent the current health.*/
	UPROPERTY(VisibleAnywhere, meta = (BindWidget, DisplayName = "Health Bar"))
	TObjectPtr<UProgressBar> DelayedHealthBar{ nullptr };

	static constexpr float s_HealthBarInterpSpeed{ 15.0f };

	/*The delay time before DelayedHealthBar starts to decrease to the actual health percentage.*/
	static constexpr float s_HealthBarDecreaseDelayDuration{ 1.0f };

	FTimerHandle m_HealthBarDecreaseDelay{};

	/* True if the DelayedHealthBar should start interpolating it's percentage to the actual health percentage */
	bool m_bIsDecreasingFill{ false };

protected:
	
	virtual void NativeConstruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override; 
};
