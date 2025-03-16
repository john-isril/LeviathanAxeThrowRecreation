// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthComponent.h"
#include <Components/WidgetComponent.h>
#include "EnemyHealthBarWidget.h"

UEnemyHealthComponent::UEnemyHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	checkf(m_HealthBarWidgetClass, TEXT("Health Bar Widget Class not set!"));

	/*Create the health bar widget and add it to the owners components.*/

	UWidgetComponent* const HealthBarWidgetComponent{ Cast<UWidgetComponent>(GetOwner()->AddComponentByClass(UWidgetComponent::StaticClass(), false, FTransform{ FQuat{}, m_HealthBarWidgetRelativeLocation, FVector{ 1.0 } }, false)) };
	HealthBarWidgetComponent->SetWidgetClass(m_HealthBarWidgetClass);
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*Store a reference to the newly created health bar widget*/
	m_HealthBarWidget = CastChecked<UEnemyHealthBarWidget>(HealthBarWidgetComponent->GetUserWidgetObject());
	m_HealthBarVisibilityDistanceSquared = FMath::Square(m_HealthBarVisibilityDistance);

	m_TargetActor = nullptr;

	SetHealthBarVisiblity(false);
	RefreshHealthBar();
}

void UEnemyHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	/*Healthbar visibility logic.*/
	if (m_TargetActor.IsValid() && !IsDead())
	{
		const double DistToTargetSquared{ FVector::DistSquared(GetOwner()->GetActorLocation(), m_TargetActor->GetActorLocation())};

		if (IsHealthbarVisible() && (DistToTargetSquared > m_HealthBarVisibilityDistanceSquared))
		{
			SetHealthBarVisiblity(false);
		}
		else if (!IsHealthbarVisible() && (DistToTargetSquared <= m_HealthBarVisibilityDistanceSquared))
		{
			SetHealthBarVisiblity(true);
		}
	}
}

void UEnemyHealthComponent::ReceiveDamage(float Damage)
{
	Super::ReceiveDamage(Damage);

	if (m_bIsDead)
	{
		m_HealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		SetHealthBarVisiblity(true);
		RefreshHealthBar();

		/*Hide the health bar after a certain duration.*/
		GetWorld()->GetTimerManager().SetTimer(m_DamageHealthBarDisplayTimer, [this]() {
			SetHealthBarVisiblity(false);
		}, m_DamageHealthBarDisplayDuration, false);
	}
}

void UEnemyHealthComponent::SetHealthBarVisiblity(bool bVisible)
{
	if (UWorld* const World{ GetWorld() })
	{
		if (World->GetTimerManager().GetTimerRemaining(m_DamageHealthBarDisplayTimer) <= 0.0f)
		{
			m_bHealthbarIsVisible = bVisible;

			if (m_HealthBarWidget)
			{
				m_HealthBarWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			}
		}
	}
	
}

void UEnemyHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(m_DamageHealthBarDisplayTimer);
}

void UEnemyHealthComponent::RefreshHealthBar()
{
	if (m_HealthBarWidget)
	{
		m_HealthBarWidget->SetPercent(m_CurrentHealth / m_MaxHealth);
	}
}
