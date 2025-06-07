// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LeviathanAxe.generated.h"

class UProjectileMovementComponent;
class URotatingMovementComponent;
class UNiagaraComponent;
class USkeletalMeshSocket;
class IAxeDamageableInterface;

DECLARE_DELEGATE(FOnFlightCompletedSignature);
DECLARE_DELEGATE_TwoParams(FOnHitSignature, AActor* HitActor, bool bPinnedActor);

UCLASS()
class AXECOMBAT_API ALeviathanAxe : public AActor
{
	GENERATED_BODY()

public:
	enum class EAxeState : uint8
	{
		EAS_None,
		/** State for when the axe is idle, but hasn't impacted with anything. Used when a character is holding the axe. */
		EAS_Idle,
		/** State after the axe has hit something after being launched by the projectile movement component. */
		EAS_IdleAfterImpact,
		/** State immediately after it has been launched by the projectile movement component. */
		EAS_PostLaunch,
		/** State for when it is flying to a specific target and is flying without assistance from the projectile movement component. */
		EAS_FlyingToTarget,
		
		EAS_Max,
	};

	/* 
	* The axe can be requested to perform certain actions when it impacts with something.
	* However, What actually happens on impact can be different from what was requested.
	*/
	enum EHitTypeRequest : uint8
	{
		EHTR_KnockBack,
		EHTR_AttachAndFreeze,
	};

public:
	ALeviathanAxe();

	virtual void PostInitializeComponents() override;
	
	virtual void Tick(float DeltaTime) override;
	
	void SetRotationRate(const FRotator& RotationRate);
	
	/*Launches the axe according to the Velocity parameter. m_ProjectileMovement will be responsible for moving the axe in this scenario until it has collided with something. HitTypeRequest will be used to influence the action taken on impact.*/
	void Launch(const FVector& Velocity, EHitTypeRequest HitTypeRequest);
	
	/*Initiates the axe to fly to a target (In this game it is the Hero's right hand for axe recalling, but can also have a different target if needed).*/
	UFUNCTION()
	void FlyToTarget(const FQuat& TargetRotation, const FVector& TargetLocation, float MinSpeed, float FlightMaxDuration);
	
	/*This should be called every frame from a moving target. In this game, the moving target is the Hero's right hand socket.*/
	void UpdateTargetRotationAndLocation(const FQuat& TargetRotation, const FVector& TargetLocation);

public:
	/** Delegate used to notify the subscriber when it has arrived at it's target */
	FOnFlightCompletedSignature OnFlightCompleted;
	/** Delegate used to notify the subscriber when it has hit something */
	FOnHitSignature OnHit;

private:
	/** Used to encapsulate all the data responsible for flying the axe to it's target */
	struct FFlightInfo
	{
		FQuat m_InitialRotation{};
		
		FQuat m_TargetRotation{};
		
		FVector m_TargetLocation{};
		
		/*Timer used to calculate the alpha value between the initial location and the target location.*/
		FTimerHandle m_FlightTimer{};

		/*Timer used to calculate the alpha value between the initial rotation and the target rotation.*/
		FTimerHandle m_InterpToRotTimer{};

		/** The radius of the flight path's curve. */
		double m_PathCurveRadius{};

		float m_Duration{};

		float m_InterpToTargetRotationDuration{};

		/*Once the distance squared between the axe and the target location is within this distance, the rotation will begin to interpolate to the target rotation.*/
		float m_StartInterpRotDistSquared{};

		bool m_bInterpingToTargetRotation{ false };
	};

private:

	UPROPERTY(VisibleAnywhere, Category = "Properties", meta = (DisplayName = "Axe Mesh"))
	TObjectPtr<UStaticMeshComponent> m_AxeMesh{ nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Properties", meta = (DisplayName = "Projectile Movement Component"))
	TObjectPtr<UProjectileMovementComponent> m_ProjectileMovementComponent{ nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Properties", meta = (DisplayName = "Rotating Movement Component"))
	TObjectPtr<URotatingMovementComponent> m_RotatingMovementComponent{ nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Properties", meta = (DisplayName = "Trail Effect"))
	TObjectPtr<UNiagaraComponent> m_TrailEffect{ nullptr };

	/*The Curve Float used to shake the axe before it's flight.*/
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (DisplayName = "Pre-Flight Shake Curve"))
	TObjectPtr<UCurveFloat> m_PreFlightShakeCurve{ nullptr };

	/** The name of the socket on the mesh used to translate the axe into the object it impacts with.*/
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (DisplayName = "Impact Socket Name"))
	FName m_ImpactSocketName{ "ImpactSocket" };

	/** Once the axe has been thrown, gravity will be applied to it once it's this distance away from it's initial location*/
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (DisplayName = "Gravity Trigger Distance"))
	float m_GravityTriggerDistance{ 750.0 };

	/* The length when performing a line trace to search for a pinnable wall*/
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (DisplayName = "Pinned Wall Search Distance"))
	float m_PinnableWallSearchDistance{ 700.0f };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Sound Attenuation"))
	TObjectPtr<USoundAttenuation> m_SoundAttenuation{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Axe Spin Sound"))
	TObjectPtr<USoundBase> m_AxeSpinSound{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Axe Impact Sound"))
	TObjectPtr<USoundBase> m_AxeImpactSound{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Audio Component"))
	TObjectPtr<UAudioComponent> m_AudioComponent{ nullptr };

	/*The Actor that the axe can be carrying.*/
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> m_CarriedActor{ nullptr };

	/*The relative rotation of the mesh before it shakes.*/
	FRotator m_PreShakeMeshRelativeRotation{};

	FHitResult m_HitResult{};
	
	FCollisionQueryParams m_CollisionQueryParams{};

	EHitTypeRequest m_HitTypeRequest{};

	/** The location of the axe before flight or before it's launched with the projectile movement component*/
	FVector m_InitialLocation{};
	
	FVector m_LocationLastFrame{};

	FFlightInfo m_FlightInfo{};
	
	FTimerHandle m_ShakeTimer{};

	float m_GravityTriggerDistanceSquared{};

	bool m_bIsShaking{ false };
	
	bool m_bMaintainConstantVelocity{ false };

	EAxeState m_State{ EAxeState::EAS_Idle };

private:
	
	bool CheckForCollision();
	
	void HandleCollision();

	/*Called when the axe hits an actor that is derived from IAxeDamageableInterface. AxeDamageableActor is the hit IAxeDamageableInterface. PreImpactVelocity is the velocity right before collision.*/
	void HandleAxeDamageableCollision(IAxeDamageableInterface* AxeDamageableActor, const FVector& PreImpactVelocity);

	/*Function called in Tick if the axe is in the IdleAfterImpact state.*/
	void IdleAfterImpactTick();
	
	/*Function called in Tick if the axe is in the PostLaunch state.*/
	void PostLaunchTick();
	
	/*
	* Rocochets off the hit object by moving at a velocity that is the previous velocity rotated by a certain degree offset.
	* The velocity before the hit, PreImpactVelocity, will help us find this new velocity.
	* The new velocity is also scaled down from the previous velocity.
	*/
	void RicochetOffHitObject(const FVector& PreImpactVelocity);
	
	/*
	* Attaches the axe to the object it has hit.
	* If the object is a skeletal mesh component, socket manipulation will be done to correctly attach the axe to the skeletons hit location.
	* Searches for the socket closest to the impact point then attaches to the socket.
	*/
	void AttachToAxeDamageableActor(IAxeDamageableInterface* AxeDamageableActor);
	
	/*Initializes the axe to begin carrying the object it has it.*/
	void CarryHitActor(const FVector& PreImpactVelocity);

	/*Function called in Tick if the axe is in the FlyingToTarget state.*/
	void FlyToTargetTick();
	
	/*Interpolates the axe's rotation to it's target rotation.*/
	void InterpToTargetRotation();
	
	/*
	* Interpolates the axe's location to it's target location along a curved path.
	* Gives the illusion that it's flying to it's target along a curved path.
	*/
	void InterpToTargetLocation();
	
	/*
	* Responsible for initiating the axe to interpolate to the target rotation.
	* Called after the m_InterpToRotationDelayTimer has completed.
	*/
	void StartInterpToTargetRotation();
	
	/*
	* Called once the m_FlightInfo.m_FlightTimer timer has completed.
	* Changes it's state and notifies all subscribers of the OnFlightCompleted delegate.
	*/
	void OnFlightTimerCompleted();

	/*
	* Callback for an IAxeDamageableInterface actor's OnFreezeCompleted delegate.
	* Responsible for making the axe fall off the actor it's attached to.
	*/
	void OnActorFreezeCompleted();

	void DeactivateInAirComponents();
	
	void PlayAudioComponent(USoundBase* Sound = nullptr);
};
