// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DELEGATE(FOnDeathSignature)
DECLARE_DELEGATE_TwoParams(FOnHealthChangeSignature, float NewHealth, float MaxHealth)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CUSTOMCOMPONENTS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();
		
	virtual void ReceiveDamage(float Damage);
	
	FORCEINLINE bool IsDead() const { return m_bIsDead; }
	
	FORCEINLINE float GetMaxHealth() const { return m_MaxHealth; }
	
	FORCEINLINE float GetCurrentHealth() const { return m_CurrentHealth; }

public:

	FOnDeathSignature OnDeath;
	
	FOnHealthChangeSignature OnHealthChange;

protected:
	virtual void BeginPlay() override;

protected:
	
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (DisplayName = "Max Health"))
	float m_MaxHealth{ 100.0f };

	float m_CurrentHealth{ m_MaxHealth };

	bool m_bIsDead{ false };
};
