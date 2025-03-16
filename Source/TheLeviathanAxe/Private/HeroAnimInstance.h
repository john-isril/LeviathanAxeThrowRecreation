// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HeroAnimInstance.generated.h"

class AHeroCharacter;

UENUM(BlueprintType)
enum class EHeroAnimationState : uint8
{
	EHAS_Idle UMETA(DisplayName = "Idle"),
	EHAS_AimingInPlace UMETA(DisplayName = "Aiming In Place"),
	EHAS_AimWalking UMETA(DisplayName = "Aim Walking"),
	EHAS_Walking UMETA(DisplayName = "Walking"),
	EHAS_Sprinting UMETA(DisplayName = "Sprinting"),
};

/**
 * 
 */
UCLASS()
class UHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Hero Owner"))
	TWeakObjectPtr<AHeroCharacter> m_HeroOwner{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Ground Speed Squared"))
	float m_GroundSpeedSquared{ 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Strafe Yaw Offset"))
	float m_StrafeYawOffset{ 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Aim In Place Yaw Offset"))
	float m_AimInPlaceYawOffset{ 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Is Equipped"))
	bool m_bIsEquipped{ true };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Is Aiming"))
	bool m_bIsAiming{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Is Turning In Place"))
	bool m_bIsTurningInPlace{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Is Recalling Axe"))
	bool m_bIsRecallingAxe{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Hero Animation State"))
	EHeroAnimationState m_HeroAnimationState{ EHeroAnimationState::EHAS_Idle };

	FRotator m_StrafeDeltaRotation{};
	FRotator m_AimInPlaceDeltaRotation{};

	static constexpr float s_DeltaRotationInterpSpeed{ 15.0f };
	static constexpr float s_AimTurnInPlaceInterpSpeed{ 2.0f };

};
