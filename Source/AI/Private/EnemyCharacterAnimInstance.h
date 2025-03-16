// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyCharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EEnemyAnimationState : uint8
{
	EEAS_None UMETA(DisplayName = "None"),
	EEAS_IdleRun UMETA(DisplayName = "Idle Run"),
	EEAS_BeingCarried UMETA(DisplayName = "Being Carried"),
	EEAS_PinnedToWall UMETA(DisplayName = "Pinned To Wall"),
	EEAS_Dead UMETA(DisplayName = "Dead"),
	EEAS_Max UMETA(DisplayName = "Max"),
};

class AEnemyCharacter;

/**
 * 
 */
UCLASS()
class UEnemyCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:

	virtual void NativeInitializeAnimation() override;
	
	virtual void NativeUpdateAnimation(float DeltaSeconds);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, DisplayName = "Enemy Owner"))
	TWeakObjectPtr<AEnemyCharacter> m_EnemyOwner{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, DisplayName = "Enemy Animation State"))
	EEnemyAnimationState m_EnemyAnimationState{ EEnemyAnimationState::EEAS_IdleRun };

	/*Created to keep the owners current pose once they die.*/
	UPROPERTY(BlueprintReadOnly, Category = "Properties", meta = (AllowPrivateAccess = true, DisplayName = "Death Pose Snapshot Name"))
	FName m_DeathPoseSnapshotName{ "DeathPose" };

	UPROPERTY(BlueprintReadOnly, Category = "Properties", meta = (AllowPrivateAccess = true, DisplayName = "Speed Squared"))
	float m_SpeedSquared{ 0.0f };

	UPROPERTY(BlueprintReadOnly, Category = "Properties", meta = (AllowPrivateAccess = true, DisplayName = "Is Turning In Place"))
	bool m_bIsTurningInPlace{ false };

	/*The owners forward vector the previous frame.*/
	FVector m_PrevFwd{};

private:
	bool TryGetEnemyOwner();
	
	// State switching callbacks.
	
	void OnEnemyOwnerDeath();
	
	void OnEnemyOwnerCarried();
	
	void OnEnemyOwnerDropped();
	
	void OnEnemyOwnerPinnedToWall();
};
