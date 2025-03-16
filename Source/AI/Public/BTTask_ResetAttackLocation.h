// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ResetAttackLocation.generated.h"

/**
 * 
 */
UCLASS()
class AI_API UBTTask_ResetAttackLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ResetAttackLocation();

private:

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

private:
	
	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "ValidAttackLocation Blackboard Key"))
	FBlackboardKeySelector m_ValidAttackLocationBBKey{};
};
