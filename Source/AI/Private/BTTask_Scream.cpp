// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Scream.h"
#include "EnemyCharacter.h"

EBTNodeResult::Type UBTTask_Scream::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyCharacter* const SelfEnemyCharacter{ EnemySetup(OwnerComp) };

	if (SelfEnemyCharacter)
	{
		SelfEnemyCharacter->Scream();

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}
