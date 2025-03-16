// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthComponent.h"
#include "EnemyHealthComponent.generated.h"

class UEnemyHealthBarWidget;

/**
 * 
 */
UCLASS()
class AI_API UEnemyHealthComponent : public UHealthComponent
{
	GENERATED_BODY()

public:

	UEnemyHealthComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void ReceiveDamage(float Damage) override;
	
	void SetHealthBarVisiblity(bool bVisible);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	FORCEINLINE bool IsHealthbarVisible() const { return m_bHealthbarIsVisible; }
	FORCEINLINE void SetTargetActor(AActor* TargetActor) { m_TargetActor = TargetActor; }

private:

	/* The widget inside of the UWidgetComponent that will be attached to the owning actor. */
	UPROPERTY(Transient)
	TObjectPtr<UEnemyHealthBarWidget> m_HealthBarWidget{ nullptr };

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Health Bar Widget Class"), Category = "Properties")
	TSubclassOf<UUserWidget> m_HealthBarWidgetClass{};

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Health Bar Widget Relative Location"), Category = "Properties")
	FVector m_HealthBarWidgetRelativeLocation{0.0, 0.0, 100.0};

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Health Bar Visibility To Target Actor Distance"))
	float m_HealthBarVisibilityDistance{ 500.0f };

	float m_HealthBarVisibilityDistanceSquared{ FMath::Square(m_HealthBarVisibilityDistance) };

	/*After receiving damage, display the health bar for this amount of seconds.*/
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Damage Health Bar DisplayDuration"))
	float m_DamageHealthBarDisplayDuration{ 5.0f };

	/*If there is a target actor, the health bar will be displayed when the parent is within a certain distance from the target actor.*/
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> m_TargetActor{ nullptr };

	FTimerHandle m_DamageHealthBarDisplayTimer;

	bool m_bHealthbarIsVisible{ false };

private:

	void RefreshHealthBar();

protected:

	virtual void BeginPlay() override;
};
