// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAnimInstance.h"
#include "HeroCharacter.h"
#include <Kismet/KismetMathLibrary.h>

void UHeroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	m_HeroOwner = CastChecked<AHeroCharacter>(TryGetPawnOwner());
	m_GroundSpeedSquared = 0.0f;
	m_StrafeYawOffset = 0.0;
	m_AimInPlaceYawOffset = 0.0;
	m_StrafeDeltaRotation = {};
	m_AimInPlaceDeltaRotation = {};
	m_bIsEquipped = false;
	m_bIsAiming = false;
	m_HeroAnimationState = EHeroAnimationState::EHAS_Idle;
}

void UHeroAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!m_HeroOwner.IsValid())
	{
		m_HeroOwner = Cast<AHeroCharacter>(TryGetPawnOwner());

		if (!m_HeroOwner.IsValid())
		{
			return;
		}
	}

	const EHeroState HeroState{ m_HeroOwner->GetHeroState() };

	switch (HeroState)
	{
	case EHeroState::EHS_Idle:
		m_HeroAnimationState = EHeroAnimationState::EHAS_Idle;
		break;

	case EHeroState::EHS_Walking:
		m_HeroAnimationState = EHeroAnimationState::EHAS_Walking;
		break;

	case EHeroState::EHS_Sprinting:
		m_HeroAnimationState = EHeroAnimationState::EHAS_Sprinting;
		break;

	case EHeroState::EHS_AimingInPlace:
		m_HeroAnimationState = EHeroAnimationState::EHAS_AimingInPlace;
		break;

	case EHeroState::EHS_AimWalking:
		m_HeroAnimationState = EHeroAnimationState::EHAS_AimWalking;
		break;

	default:
		break;
	}

	const FVector HeroVelocity{ m_HeroOwner->GetVelocity() };
	const FRotator AimRotation{ m_HeroOwner->GetBaseAimRotation() };
	
	m_GroundSpeedSquared = HeroVelocity.SizeSquared2D();

	// Strafing calculations
	{
		const FRotator MovementRotation{ UKismetMathLibrary::MakeRotFromX(HeroVelocity) };
		const FRotator DeltaRotation{ UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation) };
		// Used to smoothly interp the rotation, preventing the jump from -180 to 180 and vice-versa for the yaw.
		// Ensure the interpolation/smoothing time in the blendspace is set to 0 (should be 0 by default).
		m_StrafeDeltaRotation = FMath::RInterpTo(m_StrafeDeltaRotation, DeltaRotation, DeltaSeconds, s_DeltaRotationInterpSpeed);
		m_StrafeYawOffset = m_StrafeDeltaRotation.Yaw;
	}

	m_bIsEquipped = m_HeroOwner->IsEquipped();
	m_bIsAiming = m_HeroOwner->IsAiming();
	m_bIsRecallingAxe = m_HeroOwner->IsRecallingAxe();
	m_bIsTurningInPlace = false;

	// Aiming in place calculations.
	if (m_bIsAiming)
	{
		const FRotator DeltaRotation{ UKismetMathLibrary::NormalizedDeltaRotator(m_HeroOwner->GetActorRotation(), AimRotation)};
		
		if (!FMath::IsNearlyZero(DeltaRotation.Yaw, 0.05))
		{
			m_bIsTurningInPlace = true;
		}

		m_AimInPlaceDeltaRotation = FMath::RInterpTo(m_AimInPlaceDeltaRotation, DeltaRotation, DeltaSeconds, s_AimTurnInPlaceInterpSpeed);
		m_AimInPlaceYawOffset = m_AimInPlaceDeltaRotation.Yaw;
	}
}

