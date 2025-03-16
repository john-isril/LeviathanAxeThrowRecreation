// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckAttackLocation.generated.h"

/**
 * 
 */
UCLASS()
class AI_API UBTService_CheckAttackLocation : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CheckAttackLocation();

private:
	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "AttackMaxRadius Blackboard Key"))
	FBlackboardKeySelector m_AttackMaxRadiusBBKey{};

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "AttackLocation Blackboard Key"))
	FBlackboardKeySelector m_AttackLocationBBKey{};

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "TargetActor Blackboard Key"))
	FBlackboardKeySelector m_TargetActorBBKey{};

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "ValidAttackLocation Blackboard Key"))
	FBlackboardKeySelector m_ValidAttackLocationBBKey{};

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> m_TargetActor{ nullptr };

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "Sphere Trace Radius"))
	float m_SphereTraceRadius{ 10.0f };

	float m_AttackMaxRadiusSquared{};

protected:

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
