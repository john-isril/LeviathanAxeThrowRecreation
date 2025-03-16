// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ResetAttackLocation.h"
#include <BehaviorTree/BlackboardComponent.h>

UBTTask_ResetAttackLocation::UBTTask_ResetAttackLocation()
{
	m_ValidAttackLocationBBKey.SelectedKeyName = FName{ "ValidAttackLocation" };
}

EBTNodeResult::Type UBTTask_ResetAttackLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* const BBComp{ OwnerComp.GetBlackboardComponent() };
	
	if (!ensureAlwaysMsgf(BBComp, TEXT("Blackboard isn't set on the behavior tree!"))) return EBTNodeResult::Failed;
	
	BBComp->SetValueAsBool(m_ValidAttackLocationBBKey.SelectedKeyName, true);

	return EBTNodeResult::Succeeded;
}
