// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StartAttacking.generated.h"

class AEnemyCharacter;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class AI_API UBTTask_StartAttacking : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_StartAttacking();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "SelfActor Blackboard Key"))
	FBlackboardKeySelector m_SelfActorBBKey{};

};
