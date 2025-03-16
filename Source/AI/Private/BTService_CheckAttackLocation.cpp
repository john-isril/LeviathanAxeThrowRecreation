// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckAttackLocation.h"
#include <BehaviorTree/BlackboardComponent.h>
#include "EnemyCharacter.h"
#include <AIController.h>

UBTService_CheckAttackLocation::UBTService_CheckAttackLocation()
{
	Interval = 0.75f;
	RandomDeviation = 0.25f;
	m_AttackMaxRadiusBBKey.SelectedKeyName = FName{ "AttackMaxRadius" };
	m_AttackLocationBBKey.SelectedKeyName = FName{ "AttackLocation" };
	m_TargetActorBBKey.SelectedKeyName = FName{ "TargetActor" };
	m_ValidAttackLocationBBKey.SelectedKeyName = FName{ "ValidAttackLocation" };
}

void UBTService_CheckAttackLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* const BBComp{ OwnerComp.GetBlackboardComponent() };
	if (!ensureAlwaysMsgf(BBComp, TEXT("Blackboard is missing on the behavior tree!"))) return;

	const float AttackMaxRadius{ BBComp->GetValueAsFloat(m_AttackMaxRadiusBBKey.SelectedKeyName) };
	m_AttackMaxRadiusSquared = AttackMaxRadius * AttackMaxRadius;

	UObject* const TargetObject{ BBComp->GetValueAsObject(m_TargetActorBBKey.SelectedKeyName) };
	if (!ensureAlwaysMsgf(TargetObject, TEXT("Unable to retrieve TargetActor!"))) return;

	m_TargetActor = Cast<AActor>(TargetObject);
	if (!ensureAlwaysMsgf(TargetObject, TEXT("TargetActor is not of type Actor!"))) return;

	const FVector AttackLocation{ BBComp->GetValueAsVector(m_AttackLocationBBKey.SelectedKeyName) };

	const FVector TargetActorLocation{ m_TargetActor->GetActorLocation() };

	const bool bIsWithinAttackRadius{ FVector::DistSquared(AttackLocation, TargetActorLocation) <= m_AttackMaxRadiusSquared };

	if (!bIsWithinAttackRadius)
	{
		BBComp->SetValueAsBool(m_ValidAttackLocationBBKey.SelectedKeyName, bIsWithinAttackRadius);

		AAIController* const AIController{ OwnerComp.GetAIOwner() };

		if (AIController)
		{
			AEnemyCharacter* const EnemyOwner{ CastChecked<AEnemyCharacter>(AIController->GetPawn()) };
			
			EnemyOwner->StartPursuing();
			
			return;
		}
	}

	FHitResult HitResult{};
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(m_TargetActor.Get());
	GetWorld()->SweepSingleByChannel(HitResult, TargetActorLocation, AttackLocation, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(m_SphereTraceRadius), Params);
	
	if (HitResult.bBlockingHit)
	{
		BBComp->SetValueAsBool(m_ValidAttackLocationBBKey.SelectedKeyName, bIsWithinAttackRadius);
	}
}
