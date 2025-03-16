// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	m_CurrentHealth = m_MaxHealth;
	
	m_bIsDead = false;
}

void UHealthComponent::ReceiveDamage(float Damage)
{
	if (!m_bIsDead)
	{
		m_CurrentHealth = FMath::Max(0.0f, m_CurrentHealth - Damage);
		
		OnHealthChange.ExecuteIfBound(m_CurrentHealth, m_MaxHealth);

		if (FMath::IsNearlyZero(m_CurrentHealth))
		{
			m_bIsDead = true;
			OnDeath.ExecuteIfBound();
		}
	}
}