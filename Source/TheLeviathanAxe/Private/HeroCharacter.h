// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RandomizedMontage.h"
#include "HittableInterface.h"
#include "HeroCharacter.generated.h"

class AHeroPlayerController;
class AHeroHUD;
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UHeroAnimInstance;
class USpringArmComponent;
class UCameraComponent;
class UAxeCombatComponent;
class UAnimMontage;
class UHealthComponent;

enum class EHeroState : uint8
{
	EHS_Idle,
	EHS_AimingInPlace,
	EHS_AimWalking,
	EHS_Walking,
	EHS_Sprinting,
};

enum class EHeroFlags : uint8
{
	EHF_None = 0,
	EHF_Equipped = 1,
	EHF_RecallingAxe = 1 << 1,
	EHF_Stunned = 1 << 2,
};
ENUM_CLASS_FLAGS(EHeroFlags);

enum class EHeroInputFlags : uint8
{
	EHIF_None = 0,
	EHIF_SprintButtonDown = 1,
	EHIF_AimButtonDown = 1 << 1,
};
ENUM_CLASS_FLAGS(EHeroInputFlags);


UCLASS()
class AHeroCharacter : public ACharacter, public IHittableInterface
{
	GENERATED_BODY()

public:

	AHeroCharacter();

	virtual void PostInitializeComponents() override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void Tick(float DeltaTime) override;

	void PLayAxeThrowAnimMontage(bool bHeavy);
	
	bool IsAnyMontagePlaying() const;
	
	bool IsAiming() const;
	
	FORCEINLINE EHeroState GetHeroState() const { return m_HeroState; }
	
	FORCEINLINE const FName& GetRightHandSocketName() const { return m_RightHandSocketName; }
	
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return m_CameraBoom; }
	
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return m_FollowCamera; }
	
	FORCEINLINE UAnimMontage* GetAxeThrowAnimMontage() const { return m_AxeThrowMontage; }
	
	FORCEINLINE bool IsEquipped() const { return static_cast<bool>(m_HeroFlags & EHeroFlags::EHF_Equipped); }
	
	FORCEINLINE bool IsRecallingAxe() const { return static_cast<bool>(m_HeroFlags & EHeroFlags::EHF_RecallingAxe); }

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UPROPERTY()
	TWeakObjectPtr<AHeroPlayerController> m_HeroPlayerController{ nullptr };

	UPROPERTY()
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> m_EnhancedInputLocalPlayerSubsystem{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Hero Input Mapping Context"))
	TObjectPtr<UInputMappingContext> m_Hero_IMC{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Move Input Action"))
	TObjectPtr<UInputAction> m_Move_IA{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Look Input Action"))
	TObjectPtr<UInputAction> m_Look_IA{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Sprint Input Action"))
	TObjectPtr<UInputAction> m_Sprint_IA{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Aim Input Action"))
	TObjectPtr<UInputAction> m_Aim_IA{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Heavy Attack Input Action"))
	TObjectPtr<UInputAction> m_HeavyAttack_IA{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Light Attack Input Action"))
	TObjectPtr<UInputAction> m_LightAttack_IA{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Recall Axe Input Action"))
	TObjectPtr<UInputAction> m_RecallAxe_IA{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Pause Input Action"))
	TObjectPtr<UInputAction> m_Pause_IA{ nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (DisplayName = "Camera Boom"))
	TObjectPtr<USpringArmComponent> m_CameraBoom{ nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (DisplayName = "Follow Camera"))
	TObjectPtr<UCameraComponent> m_FollowCamera{ nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (DisplayName = "Axe Combat Component"))
	TObjectPtr<UAxeCombatComponent> m_AxeCombatComponent{ nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (DisplayName = "Health Component"))
	TObjectPtr<UHealthComponent> m_HealthComponent{ nullptr };

	/** The name of the socket that should be placed where the character mesh is holding the axe. */
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (DisplayName = "Right Hand Socket Name"))
	FName m_RightHandSocketName{ "RightHandSocket" };

	UPROPERTY()
	TObjectPtr<UHeroAnimInstance> m_AnimInstance{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Hit React Montage Info"))
	FRandomizedMontageInfo m_HitReactMontageInfo{};

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Death Montage Info"))
	FRandomizedMontageInfo m_DeathMontageInfo{};

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Axe Throw Animation Montage"))
	TObjectPtr<UAnimMontage> m_AxeThrowMontage{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Axe Catch Animation Montage"))
	TObjectPtr<UAnimMontage> m_AxeCatchMontage{ nullptr };

	/** The montage section name that is aligned with the heavy throw animation inside of the axe throw montage.*/
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Axe Throw Animation Montage Heavy Section Name"))
	FName m_HeavyAxeThrowAnimMontageSectionName{ TEXT("Heavy") };

	/** The montage section name that is aligned with the light throw animation inside of the axe throw montage.*/
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Axe Throw Animation Montage Light Section Name"))
	FName m_LightAxeThrowAnimMontageSectionName{ TEXT("Light") };

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (DisplayName = "Max Walk Speed"))
	float m_MaxWalkSpeed{ 300.0f };

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (DisplayName = "Max Aim Walk Speed"))
	float m_MaxAimWalkSpeed{ 150.0f };

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (DisplayName = "Max Sprint Speed"))
	float m_MaxSprintSpeed{ 600.0f };

	UPROPERTY(EditAnywhere, Category = "Effects", meta = (DisplayName = "Blood Splatter Particle"))
	TObjectPtr<UParticleSystem> m_BloodSplatterParticle{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Effects", meta = (DisplayName = "Blood Splatter Particle Scale"))
	float m_BloodSplatterParticleScale{ 3.0f };

	EHeroState m_HeroState{ EHeroState::EHS_Idle };
	
	EHeroFlags m_HeroFlags{ EHeroFlags::EHF_Equipped };
	
	EHeroInputFlags m_HeroInputFlags{ EHeroInputFlags::EHIF_None };

private:

	// Input functions
	FORCEINLINE bool AimButtonDown() const { return static_cast<bool>(m_HeroInputFlags & EHeroInputFlags::EHIF_AimButtonDown); }
	
	FORCEINLINE bool SprintButtonDown() const { return static_cast<bool>(m_HeroInputFlags & EHeroInputFlags::EHIF_SprintButtonDown); }

	void OnMoveStarted(const FInputActionValue& Value);
	
	void OnMoveTriggered(const FInputActionValue& Value);
	
	void OnMoveCompleted(const FInputActionValue& Value);
	
	void OnLookTriggered(const FInputActionValue& Value);
	
	void OnSprintStarted(const FInputActionValue& Value);
	
	void OnSprintCompleted(const FInputActionValue& Value);
	
	void OnAimStarted(const FInputActionValue& Value);
	
	void OnAimCompleted(const FInputActionValue& Value);
	
	void OnHeavyAttackTriggered(const FInputActionValue& Value);
	
	void OnLightAttackTriggered(const FInputActionValue& Value);
	
	void OnRecallAxeTriggered(const FInputActionValue& Value);
	
	void OnPause(const FInputActionValue& Value);

	// State switching functions
	
	void SetStateIdle();
	
	void SetStateWalking();
	
	void SetStateAimingInPlace();
	
	void SetStateAimWalking();
	
	void SetStateSprinting();
	
	// Callbacks
	
	void OnAxeThrown();
	
	void OnAxeRecallCompleted();
	
	void OnDeath();

	UFUNCTION(BlueprintCallable)
	void OnHitReactCompleted();

	UFUNCTION(BlueprintCallable)
	void OnDeathAnimCompleted();

	// Combat
	
	FORCEINLINE bool Stunned() const { return static_cast<bool>(m_HeroFlags & EHeroFlags::EHF_Stunned); }
	
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void ReactToHit(const FHitResult& HitResult, const AActor* const HitCauser) override;
	
	virtual FVector GetPawnViewLocation() const override;

protected:
	
	virtual void BeginPlay() override;

};
