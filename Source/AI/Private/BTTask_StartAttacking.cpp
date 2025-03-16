// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_StartAttacking.h"
#include <BehaviorTree/BlackboardComponent.h>
#include "EnemyCharacter.h"

UBTTask_StartAttacking::UBTTask_StartAttacking()
{
	m_SelfActorBBKey.SelectedKeyName = FName{ "SelfActor" };
}

EBTNodeResult::Type UBTTask_StartAttacking::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* const BBComp{ OwnerComp.GetBlackboardComponent() };
	if (!ensureAlwaysMsgf(BBComp, TEXT("Blackboard isn't set on the behavior tree!"))) return EBTNodeResult::Failed;

	UObject* const SelfObject{ BBComp->GetValueAsObject(m_SelfActorBBKey.SelectedKeyName) };
	if (!ensureAlwaysMsgf(SelfObject, TEXT("Unable to retrieve SelfActor!"))) return EBTNodeResult::Failed;

	AEnemyCharacter* const SelfEnemyCharacter{ Cast<AEnemyCharacter>(SelfObject) };
	if (!ensureAlwaysMsgf(SelfEnemyCharacter, TEXT("SelfActor is not of type AEnemyCharacter!"))) return EBTNodeResult::Failed;

	SelfEnemyCharacter->StartAttacking();

	return EBTNodeResult::Succeeded;
}
