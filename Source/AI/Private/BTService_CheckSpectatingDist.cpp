// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckSpectatingDist.h"
#include <BehaviorTree/BlackboardComponent.h>
#include "EnemyCharacter.h"

UBTService_CheckSpectatingDist::UBTService_CheckSpectatingDist()
{
	m_SelfActorBBKey.SelectedKeyName = FName{ "SelfActor" };
	m_TargetActorBBKey.SelectedKeyName = FName{ "TargetActor" };
	m_ValidAttackLocationBBKey.SelectedKeyName = FName{ "ValidAttackLocation" };
}

void UBTService_CheckSpectatingDist::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* const BBComp{ OwnerComp.GetBlackboardComponent() };
	if (!ensureAlwaysMsgf(BBComp, TEXT("Blackboard isn't set on the behavior tree!"))) return;

	UObject* const SelfObject{ BBComp->GetValueAsObject(m_SelfActorBBKey.SelectedKeyName) };
	if (!ensureAlwaysMsgf(SelfObject, TEXT("Unable to retrieve SelfActor!"))) return;

	AEnemyCharacter* const SelfEnemyCharacter{ Cast<AEnemyCharacter>(SelfObject) };

	if (!ensureAlwaysMsgf(SelfEnemyCharacter, TEXT("SelfActor is not of type AEnemyCharacter!"))) return;


	UObject* const TargetObject{ BBComp->GetValueAsObject(m_TargetActorBBKey.SelectedKeyName) };
	if (!ensureAlwaysMsgf(TargetObject, TEXT("Unable to retrieve TargetActor!"))) return;

	AActor* const TargetActor{ Cast<AActor>(TargetObject) };

	if (!ensureAlwaysMsgf(TargetActor, TEXT("TargetActor is not of type Actor!"))) return;

	if (FVector::DistSquared(SelfEnemyCharacter->GetActorLocation(), TargetActor->GetActorLocation()) < FMath::Square(m_MinSpectatingDistance))
	{
		BBComp->SetValueAsBool(m_ValidAttackLocationBBKey.SelectedKeyName, true);
		SelfEnemyCharacter->StartAttacking();
	}
}
