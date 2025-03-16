// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySword.generated.h"

class UBoxComponent;

UCLASS()
class AEnemySword : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySword();

	virtual void PostInitializeComponents() override;
	
	void EnableDamage(bool Enable = true);
	
	void DropFromParent();

private:
	UPROPERTY(VisibleAnywhere, Category = "Properties", meta = (DisplayName = "Mesh"))
	TObjectPtr<UStaticMeshComponent> m_Mesh{ nullptr };

	/*Collision Box that applies damage to other actors.*/
	UPROPERTY(VisibleAnywhere, Category = "Properties", meta = (DisplayName = "Damage Collision Box"))
	TObjectPtr<UBoxComponent> m_DamageCollisionBox{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Swing Sound"))
	TObjectPtr<USoundBase> m_SwingSound{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Sound Attenuation"))
	TObjectPtr<USoundAttenuation> m_SoundAttenuation{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Properties", meta = (DisplayName = "Damage Amount"))
	float m_DamageAmount{ 12.5f };

private:

	UFUNCTION()
	void OnDamageCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
