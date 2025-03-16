// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckSpectatingDist.generated.h"

/**
 * 
 */
UCLASS()
class AI_API UBTService_CheckSpectatingDist : public UBTService
{
	GENERATED_BODY()

public:

	UBTService_CheckSpectatingDist();

private:

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "SelfActor Blackboard Key"))
	FBlackboardKeySelector m_SelfActorBBKey{};

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "TargetActor Blackboard Key"))
	FBlackboardKeySelector m_TargetActorBBKey{};

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "ValidAttackLocation Blackboard Key"))
	FBlackboardKeySelector m_ValidAttackLocationBBKey{};

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "Minimum Spectating Distance"))
	float m_MinSpectatingDistance{ 300.0f };

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
