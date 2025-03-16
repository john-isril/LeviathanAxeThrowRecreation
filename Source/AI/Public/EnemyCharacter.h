// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RandomizedMontage.h"
#include <BehaviorTree/BehaviorTreeTypes.h>
#include "AxeDamageableInterface.h"
#include "EnemyCharacter.generated.h"

class AAIController;
class UPawnSensingComponent;
class AEnemySword;
class UEnemyHealthComponent;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_PursuingTargetActor UMETA(DisplayName = "Pursuing Target Actor"),
	EES_AttackingTargetActor UMETA(DisplayName = "Attacking Target Actor"),
	EES_Spectating UMETA(DisplayName = "Spectating"),
	EES_Stunned UMETA(DisplayName = "Stunned"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_None UMETA(DisplayName = "None"),
};

/*
* Struct for encapsulating data for the hit react montage.
*/
USTRUCT(BlueprintType)
struct FHitReactMontageInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Montage"))
	TObjectPtr<UAnimMontage> m_Montage{ nullptr };

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Hit React Front Section Name"))
	FName m_FrontSectionName{ "HitFront" };

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Hit React Back Section Name"))
	FName m_BackSectionName{ "HitBack" };

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Hit React Left Section Name"))
	FName m_LeftSectionName{ "HitLeft" };

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Hit React Right Section Name"))
	FName m_RightSectionName{ "HitRight" };
};

DECLARE_DELEGATE(FOnFreezeSignature)
DECLARE_DELEGATE(FOnActionCompletedSignature)
DECLARE_DELEGATE(FOnCarriedSignature);
DECLARE_DELEGATE(FOnDroppedSignature);
DECLARE_DELEGATE(FOnPinnedToWallSignature);
DECLARE_MULTICAST_DELEGATE(FOnDeathDelegateSignature);

UCLASS()
class AI_API AEnemyCharacter : public ACharacter, public IAxeDamageableInterface
{
	GENERATED_BODY()

public:

	AEnemyCharacter();

	virtual void PostInitializeComponents() override;
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/*Applies damage to health.*/
	float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	/*Determines how the enemy will react to a hit. Returns true if there was a reaction, false if the EnemyCharacter is already dead (does not apply damage to health).*/
	virtual bool OnHit(const FHitResult& HitResult, AActor* Causer, EAxeHitType AxeHitType) override;
	
	virtual void Freeze() override;
	
	virtual void Unfreeze() override;
	
	virtual USkeletalMeshComponent* GetSkeletalMeshComponent() const override;
	
	virtual bool IsAlive() const override;

	/*Initiates the movement towards the target actor.*/
	void StartPursuing();
	
	void StartAttacking();

	void Scream();
	
	void Attack();

	UFUNCTION(BlueprintCallable)
	void OnScreamMontageCompleted();

	UFUNCTION(BlueprintCallable)
	void OnAttackMontageCompleted();

	UFUNCTION(BlueprintCallable)
	void OnHitReactMontageCompleted();

	UFUNCTION(BlueprintCallable)
	void ActivateWeaponDamage(bool Activate);

	UFUNCTION(BlueprintCallable)
	void OnDeathCompleted(bool bRagdollDeath = false);

	FORCEINLINE EEnemyState GetEnemyState() const { return m_EnemyState; }
	
	FORCEINLINE bool CanMoveWhilePatrolling() const { return m_bCanMoveWhilePatrolling; }
	
	FORCEINLINE float GetFreezeMaxDuration() const { return m_FreezeMaxDuration; };

public:
	
	/*Required to notify the behavior tree.*/
	FOnActionCompletedSignature OnActionCompleted;
	
	FOnFreezeSignature OnFreeze;
	
	FOnCarriedSignature OnCarried;
	
	FOnDroppedSignature OnDropped;
	
	FOnPinnedToWallSignature OnPinnedToWall;
	
	FOnDeathDelegateSignature OnDeathDelegate;

private:
	
	UPROPERTY()
	TObjectPtr<AAIController> m_AIController{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (DisplayName = "Walk Speed"))
	float m_WalkSpeed{ 100.0f };

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (DisplayName = "Run Speed"))
	float m_RunSpeed{ 200.0f };

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (DisplayName = "Attacking Run Speed"))
	float m_AttackingRunSpeed{ 300.0f };

	UPROPERTY(VisibleAnywhere, Category = "AI", meta = (DisplayName = "Pawn Sensing Component"))
	TObjectPtr<UPawnSensingComponent> m_PawnSensingComponent{ nullptr };

	/* When patrolling, this instance can move to different patrol locations rather than staying at a single location. */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "Can Move While Patrolling"))
	bool m_bCanMoveWhilePatrolling{ false };

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "CanMoveWhilePatrolling Blackboard Key Name"))
	FName m_CanMoveWhilePatrollingBBKeyName{ "CanMoveWhilePatrolling" };

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "TargetActor Blackboard Key Name"))
	FName m_TargetActorBBKeyName{ "TargetActor" };

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "EnemyState Blackboard Key Name"))
	FName m_EnemyStateBBKeyName{ "EnemyState" };

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "AttackMaxRadius Blackboard Key Name"))
	FName m_AttackMaxRadiusBBKeyName{ "AttackMaxRadius" };

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "Attack Max Radius"))
	float m_AttackMaxRadius{ 350.0f };

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Scream Montage Info"))
	FRandomizedMontageInfo m_ScreamMontageInfo;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Attack Montage Info"))
	FRandomizedMontageInfo m_AttackMontageInfo;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Hit React Montage Info"))
	FHitReactMontageInfo m_HitReactMontageInfo;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Death Montage Info"))
	FRandomizedMontageInfo m_DeathMontageInfo;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Sword Class"))
	TSubclassOf<AEnemySword> m_EnemySwordClass;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Enemy Health Component"))
	TObjectPtr<UEnemyHealthComponent> m_EnemyHealthComponent{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "FreezeTime Material Parameter Name"))
	FName m_FreezeTimeMaterialParameterName{ "FreezeTime" };

	/*The enemy character can only be frozen within this duration.*/
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Freeze Max Duration"))
	float m_FreezeMaxDuration{ 10.0f };

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Death Duration"))
	float m_DeathDuration{ 5.0f };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Growl Sound"))
	TObjectPtr<USoundBase> m_GrowlSound{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Scream Sound"))
	TObjectPtr<USoundBase> m_ScreamSound{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Attack Sound"))
	TObjectPtr<USoundBase> m_AttackSound{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Hit Sound"))
	TObjectPtr<USoundBase> m_HitSound{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Sound Attenuation"))
	TObjectPtr<USoundAttenuation> m_SoundAttenuation{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Loud Sound Attenuation"))
	TObjectPtr<USoundAttenuation> m_LoudSoundAttenuation{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Growl Delay Min Duration"))
	float m_GrowlDelayMinDuration{ 3.0f };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Growl Delay Max Duration"))
	float m_GrowlDelayMaxDuration{ 10.0f };

	UPROPERTY()
	TObjectPtr<AEnemySword> m_Sword{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Properties", meta = (DisplayName = "Weapon Socket Name"))
	FName m_WeaponSocketName{ "WeaponSocket" };

	UPROPERTY(EditAnywhere, Category = "Effects", meta = (DisplayName = "Blood Splatter Particle"))
	TObjectPtr<UParticleSystem> m_BloodSplatterParticle{ nullptr };

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> m_TargetActor{ nullptr };

	EEnemyState m_EnemyState{ EEnemyState::EES_Patrolling };
	
	EEnemyState m_PreStunnedEnemyState{ EEnemyState::EES_None };

	FTimerHandle m_GrowlDelayTimer;
	
	FTimerHandle m_FreezeTimer;

	float m_AttackMaxRadiusSquared{ m_AttackMaxRadius * m_AttackMaxRadius };
	
	bool m_bIsTurningToTarget{ false };

	/*Only this number of enemies can attack simultaneously.*/
	static constexpr uint8 s_MaxAttackingEnemies{ 2 };
	
	inline static TSet<AEnemyCharacter*> s_AllEnemies{};
	
	inline static TSet<AEnemyCharacter*> s_AttackingEnemies{};

private:

	void OnGrowlDelayTimerCompleted();

	/* Alert all enemies that the target actor has been found */
	UFUNCTION()
	void OnSeePawn(APawn* Pawn);

	void AlertAllEnemies(AActor* TargetActor, bool bIgnoreSelf);

	/*Will set m_bIsTurningToTarget to true if it should start facing the target and will provide the direction to the target.*/
	void CheckTurnToTarget(FVector& DirectionToTarget);

	/*Faces the enemy towards it's target.*/
	void TurnToTarget(const FVector &TargetDirection, float DeltaTime);
	
	/*A directional reaction to a hit. This will have the enemy move towards the hit direction.*/
	void GetKnockedBack(const FVector& HitDirection);
	
	void EnterStunnedState();

	void RecoverFromStunnedState();
	
	void OnDeathStarted();

	FORCEINLINE bool AttackingEnemiesAtMaxCapacity() const { return (s_AttackingEnemies.Num() == s_MaxAttackingEnemies); }

protected:

	virtual void BeginPlay() override;
};
