// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BTTask_EnemyAction.h"
#include "BTTask_Scream.generated.h"

/**
 * 
 */
UCLASS()
class AI_API UBTTask_Scream : public UBTTask_EnemyAction
{
	GENERATED_BODY()

private:

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
