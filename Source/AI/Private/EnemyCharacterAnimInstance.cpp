// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacterAnimInstance.h"
#include "EnemyCharacter.h"

void UEnemyCharacterAnimInstance::NativeInitializeAnimation()
{
	m_SpeedSquared = 0.0f;
	m_bIsTurningInPlace = false;
	m_EnemyAnimationState = EEnemyAnimationState::EEAS_IdleRun;

	TryGetEnemyOwner();
}

void UEnemyCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (m_EnemyOwner.IsExplicitlyNull())
	{
		if (!TryGetEnemyOwner()) return;
	}

	m_SpeedSquared = m_EnemyOwner->GetVelocity().SizeSquared2D();
	
	const FVector CurrFwd{ m_EnemyOwner->GetActorForwardVector() };

	m_bIsTurningInPlace = false;

	/*If there is no speed and there is a delta between the current and previous forward, then the character is turning in place.*/
	if (FMath::IsNearlyZero(m_SpeedSquared))
	{
		const double FwdProjPrevFwd{ FMath::Abs(FVector::DotProduct(CurrFwd, m_PrevFwd)) };

		if (!FMath::IsNearlyEqual(FwdProjPrevFwd, 1.0))
		{
			m_bIsTurningInPlace = true;
		}
	}

	m_PrevFwd = CurrFwd;
}

bool UEnemyCharacterAnimInstance::TryGetEnemyOwner()
{
	APawn* const PawnOwner{ TryGetPawnOwner() };

	if (PawnOwner)
	{
		m_EnemyOwner = CastChecked<AEnemyCharacter>(PawnOwner);
		
		/*Subscribe to the enemy owners state switching delegates.*/
		m_EnemyOwner->OnDeathDelegate.AddUObject(this, &UEnemyCharacterAnimInstance::OnEnemyOwnerDeath);
		m_EnemyOwner->OnCarried.BindUObject(this, &UEnemyCharacterAnimInstance::OnEnemyOwnerCarried);
		m_EnemyOwner->OnPinnedToWall.BindUObject(this, &UEnemyCharacterAnimInstance::OnEnemyOwnerPinnedToWall);
		m_EnemyOwner->OnDropped.BindUObject(this, &UEnemyCharacterAnimInstance::OnEnemyOwnerDropped);
		
		if (m_EnemyOwner.IsValid()) return true;
	}

	return false;
}

void UEnemyCharacterAnimInstance::OnEnemyOwnerDeath()
{	
	SavePoseSnapshot(m_DeathPoseSnapshotName);
	StopAllMontages(0.0f);
	m_EnemyAnimationState = EEnemyAnimationState::EEAS_Dead;
}

void UEnemyCharacterAnimInstance::OnEnemyOwnerCarried()
{
	m_EnemyAnimationState = EEnemyAnimationState::EEAS_BeingCarried;
}

void UEnemyCharacterAnimInstance::OnEnemyOwnerDropped()
{
	m_EnemyAnimationState = EEnemyAnimationState::EEAS_IdleRun;
}

void UEnemyCharacterAnimInstance::OnEnemyOwnerPinnedToWall()
{
	m_EnemyAnimationState = EEnemyAnimationState::EEAS_PinnedToWall;
}
