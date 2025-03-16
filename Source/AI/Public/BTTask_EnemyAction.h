// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_EnemyAction.generated.h"

class AEnemyCharacter;

/**
 * 
 */
UCLASS()
class AI_API UBTTask_EnemyAction : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	UBTTask_EnemyAction();

protected:
	
	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "SelfActor Blackboard Key"))
	FBlackboardKeySelector m_SelfActorBBKey;

protected:

	AEnemyCharacter* const EnemySetup(UBehaviorTreeComponent& OwnerComp) const;
};
