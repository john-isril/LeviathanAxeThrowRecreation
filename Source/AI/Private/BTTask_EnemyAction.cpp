// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_EnemyAction.h"
#include <BehaviorTree/BlackboardComponent.h>
#include "EnemyCharacter.h"

UBTTask_EnemyAction::UBTTask_EnemyAction()
{
	m_SelfActorBBKey.SelectedKeyName = FName{ "SelfActor" };
}

AEnemyCharacter* const UBTTask_EnemyAction::EnemySetup(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* const BBComp{ OwnerComp.GetBlackboardComponent() };
	if (!ensureAlwaysMsgf(BBComp, TEXT("Blackboard isn't set on the behavior tree!"))) return nullptr;

	UObject* const SelfObject{ BBComp->GetValueAsObject(m_SelfActorBBKey.SelectedKeyName) };
	if (!ensureAlwaysMsgf(SelfObject, TEXT("Unable to retrieve SelfActor!"))) return nullptr;

	AEnemyCharacter* const SelfEnemyCharacter{ Cast<AEnemyCharacter>(SelfObject) };
	if (!ensureAlwaysMsgf(SelfEnemyCharacter, TEXT("SelfActor is not of type AEnemyCharacter!"))) return nullptr;

	SelfEnemyCharacter->OnActionCompleted.BindLambda([this, &OwnerComp]()
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	});

	return SelfEnemyCharacter;
}