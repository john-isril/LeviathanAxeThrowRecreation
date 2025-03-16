// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "EnemyCharacter.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AEnemyCharacter* const SelfEnemyCharacter{ EnemySetup(OwnerComp) })
	{
		SelfEnemyCharacter->Attack();

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}
