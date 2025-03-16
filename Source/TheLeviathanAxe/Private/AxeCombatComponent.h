// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AxeCombatComponent.generated.h"

class AHeroCharacter;
class AHeroHUD;
class ALeviathanAxe;

enum class EAxeCombatFlags : uint8
{
	EACF_None = 0,
	EACF_InterpingCameraSocketOffset = 1,
	EACF_InterpingFOV = 1 << 1,
	EACF_Aiming = 1 << 2,
	EACF_RecallingAxe = 1 << 3,
};
ENUM_CLASS_FLAGS(EAxeCombatFlags);


DECLARE_DELEGATE(FOnAxeThrownSignature);
DECLARE_DELEGATE(FOnAxeRecallCompletedSignature);

/*This component is esentially the interface between the character owner and the Leviathan Axe.
* It takes responsibility for things such as spawning the axe, applying damage, etc.
*/

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UAxeCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAxeCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void AttachAxeToHand() const;
	
	void Aim() ;
	
	void StopAiming();
	
	void LightAttack() const;
	
	void HeavyAttack() const;
	
	/*Sends the axe flying back to m_Hero's right hand.*/
	void RecallAxe();

	FORCEINLINE bool IsAiming() const { return bool(m_AxeCombatFlags & EACF_Aiming); }

public:

	FOnAxeThrownSignature OnAxeThrown;
	
	FOnAxeRecallCompletedSignature OnAxeRecallCompleted;
	
	using enum EAxeCombatFlags;

private:

	UPROPERTY()
	TWeakObjectPtr<AHeroCharacter> m_HeroOwner{ nullptr };

	UPROPERTY()
	TWeakObjectPtr<AHeroHUD> m_HeroHUD{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Leviathan Axe Class"))
	TSubclassOf<ALeviathanAxe> m_LeviathanAxeClass{};

	UPROPERTY()
	TObjectPtr<ALeviathanAxe> m_LeviathanAxe{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Heavy Axe Throw Speed"))
	double m_HeavyAxeThrowSpeed{ 1500.0 };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Light Axe Throw Speed"))
	double m_LightAxeThrowSpeed{ 3500.0 };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Axe Throw Rotation Rate"))
	FRotator m_AxeThrowRotationRate{ 1440.0, 0.0, 0.0 };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Axe Recall Max Duration"))
	float m_AxeRecallMaxDuration{ 1.0 };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Axe Recall Min Speed"))
	float m_AxeRecallMinSpeed{ 1600.0f };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Light Damage Amount"))
	float m_LightDamageAmount{ 17.0f };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Heavy Damage Amount"))
	float m_HeavyDamageAmount{ 35.0f };

	/*The amount of damage applied when the Leviathan Axe pins an enemy against a pinnable object.*/
	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Pinning Damage Amount"))
	float m_PinningDamageAmount{ 100.0f };

	UPROPERTY(EditAnywhere, Category = "Leviathan Axe", meta = (DisplayName = "Axe Recall Rotation Rate"))
	FRotator m_AxeRecallRotationRate{ 720.0, 0.0, 0.0 };

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (DisplayName = "Base Camera Socket Offset"))
	FVector m_BaseCameraSocketOffset{ 0.0, 60.0, 63.0 };

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (DisplayName = "Aim Camera Socket Offset"))
	FVector m_AimCameraSocketOffset{ 0.0, 60.0, 50.0 };

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (DisplayName = "Camera Interp Speed"))
	float m_CameraInterpSpeed{ 15.0f };

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (DisplayName = "Base Field of View"))
	float m_BaseFOV{ 90.0f };

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (DisplayName = "Aim Field of View"))
	float m_AimFOV{ 70.0f };

	EAxeCombatFlags m_AxeCombatFlags{ EACF_None };

	FVector m_TargetCameraSocketOffset{ m_BaseCameraSocketOffset };
	
	float m_TargetFOV{ m_BaseFOV };
	
	float m_DamageToApply{ 0.0f };

private:

	/*Interpolates the camera socket offset for m_Hero's camera boom between aiming an non-aiming offsets.*/
	void InterpCameraSocketOffset(float DeltaTime);

	/*Interpolates m_Hero's camera field of view between aiming an non-aiming FOVs.*/
	void InterpFOV(float DeltaTime);
	
	/*This function is sets up subscriptions to anim notifies that are called during axe throws. This enables us to call the correct function right when the axe should leave the hero's hand in the anim montage.*/
	void SubscribeToAnimNotifies();
	
	/*Launches the axe.*/
	void OnAxeThrowNotified(bool bIsHeavy);

	/*Called when the axe has completed it's flight to it's target. In this case, it's when the axe recall is complete. Before the function is complete, it notifies it's subscriber (the hero) that the recall is complete. */
	void OnAxeFlightCompleted();
	
	/*Called when the Leviathan Axe has hit an object. Will apply damage if needed. bPinned actor indicates if the actor is pinned to an object. A different damage amount will be applied based on bPinnedActor.*/
	void OnAxeHit(AActor* HitActor, bool bPinnedActor);

protected:

	virtual void BeginPlay() override;
};
