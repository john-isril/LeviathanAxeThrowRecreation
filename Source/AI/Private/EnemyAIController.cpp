// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include <BehaviorTree/BehaviorTree.h>
#include <Navigation/PathFollowingComponent.h>

AEnemyAIController::AEnemyAIController()
{
	m_BehaviorTree = CreateDefaultSubobject<UBehaviorTree>(TEXT("BehaviorTree"));
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ensureAlwaysMsgf(m_BehaviorTree, TEXT("Behavior Tree is not set!")))
	{
		RunBehaviorTree(m_BehaviorTree);
	}
}
