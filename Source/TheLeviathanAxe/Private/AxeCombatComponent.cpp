// Fill out your copyright notice in the Description page of Project Settings.


#include "AxeCombatComponent.h"
#include "HeroCharacter.h"
#include "HeroHUD.h"
#include "LeviathanAxe.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include "AxeThrowAnimNotify.h"
#include <Kismet/GameplayStatics.h>

UAxeCombatComponent::UAxeCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UAxeCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	m_HeroOwner = CastChecked<AHeroCharacter>(GetOwner());

	APlayerController* const PlayerController{ CastChecked<APlayerController>(m_HeroOwner->GetController()) };
	m_HeroHUD = CastChecked<AHeroHUD>(PlayerController->GetHUD());

	m_HeroOwner->GetCameraBoom()->SocketOffset = m_BaseCameraSocketOffset;
	m_HeroOwner->GetFollowCamera()->FieldOfView = m_BaseFOV;

	if (ensureAlwaysMsgf(m_LeviathanAxeClass, TEXT("Leviathan Axe Class is not set!")))
	{
		m_LeviathanAxe = GetWorld()->SpawnActor<ALeviathanAxe>(m_LeviathanAxeClass);
		m_LeviathanAxe->SetInstigator(Cast<APawn>(GetOwner()));
		m_LeviathanAxe->OnFlightCompleted.BindUObject(this, &UAxeCombatComponent::OnAxeFlightCompleted);
		m_LeviathanAxe->OnHit.BindUObject(this, &UAxeCombatComponent::OnAxeHit);

		AttachAxeToHand();
	}

	SubscribeToAnimNotifies();

	m_AxeCombatFlags = EACF_None;
}

void UAxeCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (static_cast<bool>(m_AxeCombatFlags & EACF_InterpingCameraSocketOffset))
	{
		InterpCameraSocketOffset(DeltaTime);
	}

	if (static_cast<bool>(m_AxeCombatFlags & EACF_InterpingFOV))
	{
		InterpFOV(DeltaTime);
	}

	if (static_cast<bool>(m_AxeCombatFlags & EACF_RecallingAxe))
	{
		if (USkeletalMeshComponent* const CharacterMesh{ m_HeroOwner->GetMesh() })
		{
			const FVector HeroRightHandLocation{ CharacterMesh->GetSocketLocation(m_HeroOwner->GetRightHandSocketName()) };
			m_LeviathanAxe->UpdateTargetRotationAndLocation(m_HeroOwner->GetActorQuat(), HeroRightHandLocation);
		}
	}
}

void UAxeCombatComponent::AttachAxeToHand() const
{
	m_LeviathanAxe->AttachToComponent(m_HeroOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, m_HeroOwner->GetRightHandSocketName());
}

void UAxeCombatComponent::Aim()
{
	if (m_HeroHUD.IsValid())
	{
		m_HeroHUD->DisplayCrosshairs();
	}

	m_TargetCameraSocketOffset = m_AimCameraSocketOffset;
	m_TargetFOV = m_AimFOV;

	m_AxeCombatFlags |= EACF_InterpingCameraSocketOffset | EACF_InterpingFOV | EACF_Aiming;
}

void UAxeCombatComponent::StopAiming()
{
	m_HeroHUD->DisplayCrosshairs(false);

	m_TargetCameraSocketOffset = m_BaseCameraSocketOffset;
	m_TargetFOV = m_BaseFOV;

	m_AxeCombatFlags |= EACF_InterpingCameraSocketOffset | EACF_InterpingFOV;
	m_AxeCombatFlags &= ~EACF_Aiming;
}

void UAxeCombatComponent::LightAttack() const
{
	if (IsAiming() && !m_HeroOwner->IsAnyMontagePlaying())
	{
		m_HeroOwner->PLayAxeThrowAnimMontage(false);
	}
}

void UAxeCombatComponent::HeavyAttack() const
{
	if (IsAiming() && !m_HeroOwner->IsAnyMontagePlaying())
	{
		m_HeroOwner->PLayAxeThrowAnimMontage(true);
	}
}

void UAxeCombatComponent::RecallAxe()
{
	USkeletalMeshComponent* const HeroMesh{ m_HeroOwner->GetMesh() };

	if (!HeroMesh) return;

	UAnimInstance* const HeroAnimInstance{ HeroMesh->GetAnimInstance() };

	if (HeroAnimInstance)
	{
		constexpr float MontageBlendOut{ 0.2f };

		HeroAnimInstance->StopAllMontages(MontageBlendOut);
	}

	const FVector HeroRightHandLocation{ HeroMesh->GetSocketLocation(m_HeroOwner->GetRightHandSocketName()) };

	m_LeviathanAxe->SetRotationRate(m_AxeRecallRotationRate);

	m_DamageToApply = m_LightDamageAmount;

	m_LeviathanAxe->FlyToTarget(m_HeroOwner->GetActorQuat(), HeroRightHandLocation, m_AxeRecallMinSpeed, m_AxeRecallMaxDuration);

	m_AxeCombatFlags |= EACF_RecallingAxe;
}

void UAxeCombatComponent::InterpCameraSocketOffset(float DeltaTime)
{
	USpringArmComponent* const CameraBoom{ m_HeroOwner->GetCameraBoom() };

	if (!CameraBoom) return;

	FVector& CurrentCameraOffset{ CameraBoom->SocketOffset };
	CurrentCameraOffset = FMath::VInterpTo(CurrentCameraOffset, m_TargetCameraSocketOffset, DeltaTime, m_CameraInterpSpeed);

	const double CurrentProjTargetSocketOffset{ CurrentCameraOffset.Dot(m_TargetCameraSocketOffset) };

	if (FMath::IsNearlyEqual(CurrentProjTargetSocketOffset, 1.0))
	{
		m_AxeCombatFlags &= ~EACF_InterpingCameraSocketOffset;
	}
}

void UAxeCombatComponent::InterpFOV(float DeltaTime)
{
	UCameraComponent* const Camera{ m_HeroOwner->GetFollowCamera() };

	if (!Camera) return;

	float& CurrentFOV{ Camera->FieldOfView };
	CurrentFOV = FMath::FInterpTo(CurrentFOV, m_TargetFOV, DeltaTime, m_CameraInterpSpeed);

	if (FMath::IsNearlyEqual(CurrentFOV, m_TargetFOV))
	{
		m_AxeCombatFlags &= ~EACF_InterpingFOV;
	}
}

void UAxeCombatComponent::SubscribeToAnimNotifies()
{
	for (const FAnimNotifyEvent& Event : m_HeroOwner->GetAxeThrowAnimMontage()->Notifies)
	{
		if (UAxeThrowAnimNotify* const AxeThrowNotify{ Cast<UAxeThrowAnimNotify>(Event.Notify) })
		{
			AxeThrowNotify->OnNotified.BindUObject(this, &UAxeCombatComponent::OnAxeThrowNotified);
		}
	}
}

void UAxeCombatComponent::OnAxeThrowNotified(bool bIsHeavy)
{
	if (!GEngine || !GEngine->GameViewport) return;

	m_LeviathanAxe->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	m_LeviathanAxe->SetActorRotation(m_HeroOwner->GetControlRotation());

	if (!bIsHeavy)
	{
		m_LeviathanAxe->AddActorLocalRotation(FRotator{ 0.0, 0.0, 75.0 }); /* Offset for light throws. */
	}

	/* Get the crosshairs position and direction in world space (this method allows the crosshairs to be placed anywhere in the viewport). */
	FVector CrosshairsWorldPosition{};
	FVector CrosshairsWorldDirection{};
	UGameplayStatics::DeprojectScreenToWorld(GetWorld()->GetFirstPlayerController(), m_HeroHUD->GetCrosshairsScreenPosition(), CrosshairsWorldPosition, CrosshairsWorldDirection);

	/* We want the launch location to be in across from the crosshairs but on the right side of the character. */
	const FVector CrosshairsToAxe{ m_LeviathanAxe->GetActorLocation() - CrosshairsWorldPosition };
	const double CrosshairsToAxeProjCrosshairsDirection{ CrosshairsToAxe.Dot(CrosshairsWorldDirection) };
	const FVector TargetAxeLaunchLocation{ CrosshairsWorldPosition + CrosshairsWorldDirection * CrosshairsToAxeProjCrosshairsDirection };
	m_LeviathanAxe->SetActorLocation(TargetAxeLaunchLocation);

	m_DamageToApply = bIsHeavy ? m_HeavyDamageAmount : m_LightDamageAmount;

	/*Request for the axe to perform a specific type of action when it hits an enemy*/
	ALeviathanAxe::EHitTypeRequest ImpactActionRequest{ bIsHeavy ? ALeviathanAxe::EHTR_AttachAndFreeze : ALeviathanAxe::EHTR_KnockBack };

	FVector Velocity{ CrosshairsWorldDirection };
	Velocity *= bIsHeavy ? m_HeavyAxeThrowSpeed : m_LightAxeThrowSpeed;

	m_LeviathanAxe->Launch(Velocity, ImpactActionRequest);

	m_LeviathanAxe->SetRotationRate(m_AxeThrowRotationRate);

	/*m_LeviathanAxe->Launch(Velocity, ImpactActionRequest);*/

	OnAxeThrown.ExecuteIfBound();
}

void UAxeCombatComponent::OnAxeFlightCompleted()
{
	AttachAxeToHand();
	m_AxeCombatFlags &= ~EACF_RecallingAxe;

	OnAxeRecallCompleted.ExecuteIfBound();
}

void UAxeCombatComponent::OnAxeHit(AActor* HitActor, bool bPinnedActor)
{
	UGameplayStatics::ApplyDamage(HitActor, bPinnedActor ? m_PinningDamageAmount : m_DamageToApply, m_HeroOwner->GetController(), m_HeroOwner.Get(), UDamageType::StaticClass());
}
