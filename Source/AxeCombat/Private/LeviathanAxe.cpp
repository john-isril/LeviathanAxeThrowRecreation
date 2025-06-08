// Fill out your copyright notice in the Description page of Project Settings.


#include "LeviathanAxe.h"
#include <GameFramework/ProjectileMovementComponent.h>
#include <GameFramework/RotatingMovementComponent.h>
#include <Engine/StaticMeshSocket.h>
#include <NiagaraComponent.h>
#include <Engine/SkeletalMeshSocket.h>
#include "PinnableWall.h"
#include "AxeDamageableInterface.h"
#include <Kismet/GameplayStatics.h>
#include <Components/AudioComponent.h>

using enum ALeviathanAxe::EAxeState;

ALeviathanAxe::ALeviathanAxe()
{
	PrimaryActorTick.bCanEverTick = true;

	m_AxeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Axe Mesh"));
	SetRootComponent(m_AxeMesh);
	
	m_AxeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_AxeMesh->SetGenerateOverlapEvents(false);

	m_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	m_ProjectileMovementComponent->SetUpdatedComponent(GetRootComponent());
	m_ProjectileMovementComponent->bAutoActivate = false;

	m_RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("Rotating Movement Component"));
	m_RotatingMovementComponent->SetUpdatedComponent(GetRootComponent());
	m_RotatingMovementComponent->RotationRate = FRotator{ 1440.0, 0.0, 0.0 };
	m_RotatingMovementComponent->bAutoActivate = false;

	m_TrailEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailEffect"));
	m_TrailEffect->SetupAttachment(GetRootComponent());
	m_TrailEffect->SetRelativeRotation(FRotator{ 0.0, 0.0, 90.0 });
	m_TrailEffect->bAutoActivate = false;

	m_AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	m_AudioComponent->SetupAttachment(GetRootComponent());
}

void ALeviathanAxe::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	m_AxeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// A user parameter must be created and bound to the emitters "Ribbon Width" property in the Niagara particle system. Name it TrailHeight.
	if (m_TrailEffect)
	{
		m_TrailEffect->SetVariableFloat(FName{ "TrailHeight" }, m_AxeMesh->Bounds.BoxExtent.Z * 2.0);
	}

	if (ensureAlwaysMsgf(m_SoundAttenuation, TEXT("Sound Attenuation is not set!")))
	{
		m_AudioComponent->AttenuationSettings = m_SoundAttenuation;
	}

	m_CarriedActor.Reset();
	m_CollisionQueryParams.ClearIgnoredActors();
	m_GravityTriggerDistanceSquared = m_GravityTriggerDistance * m_GravityTriggerDistance;
	m_FlightInfo.m_StartInterpRotDistSquared = FMath::Square((m_AxeMesh->Bounds.BoxExtent.Z * 2.0) * 3.0);
	m_bMaintainConstantVelocity = false;
	m_bIsShaking = false;
	m_State = EAS_Idle;
}

void ALeviathanAxe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (m_State)
	{
	case EAS_Idle:
		break;

	case EAS_IdleAfterImpact:
		IdleAfterImpactTick();
		break;

	case EAS_PostLaunch:
		PostLaunchTick();
		break;

	case EAS_FlyingToTarget:
		FlyToTargetTick();
		break;

	default:
		break;
	}

}

void ALeviathanAxe::SetRotationRate(const FRotator& RotationRate)
{
	m_RotatingMovementComponent->RotationRate = RotationRate;
}

void ALeviathanAxe::Launch(const FVector& Velocity, EHitTypeRequest HitTypeRequest)
{
	PlayAudioComponent(m_AxeSpinSound);
	
	m_HitTypeRequest = HitTypeRequest;

	m_bMaintainConstantVelocity = false;
	
	m_RotatingMovementComponent->Activate();

	m_ProjectileMovementComponent->Velocity = Velocity;
	m_ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	m_ProjectileMovementComponent->Activate();

	m_InitialLocation = GetActorLocation();
	m_LocationLastFrame = m_InitialLocation; // Setup for collision detection later on.

	m_State = EAS_PostLaunch;

	if (m_TrailEffect)
	{
		m_TrailEffect->Activate();
	}
}

void ALeviathanAxe::FlyToTarget(const FQuat& TargetRotation, const FVector& TargetLocation, float MinSpeed, float FlightMaxDuration)
{
	m_State = EAS_FlyingToTarget;

	m_CollisionQueryParams.ClearIgnoredActors();

	AActor* const AttachedToActor{ GetAttachParentActor() };

	if (AttachedToActor)
	{
		/* Apply damage and call OnHit if's an AxeDamageableInterface.*/
		if (IAxeDamageableInterface* const AttachedAxeDamageableActor{ Cast<IAxeDamageableInterface>(AttachedToActor) })
		{
			OnHit.ExecuteIfBound(m_HitResult.GetActor(), false);
			AttachedAxeDamageableActor->OnHit(m_HitResult, GetInstigator(), EAxeHitType::EAHT_DetachFrom);
		}
		/*Start shaking the axe if it's not already shaking and if it's not carrying an actor.*/
		else if (!m_bIsShaking && m_CarriedActor.IsExplicitlyNull())
		{
			m_State = EAS_IdleAfterImpact;
			m_bIsShaking = true;
			m_PreShakeMeshRelativeRotation = m_AxeMesh->GetRelativeRotation();
			FTimerDelegate Delegate;
			/*Return back to this function after the axe is done shaking then it'll execute the rest of this function.*/
			Delegate.BindUFunction(this, "FlyToTarget", TargetRotation, TargetLocation, MinSpeed, FlightMaxDuration);
			
			static constexpr float ShakeDuration{ 0.3f };
			GetWorldTimerManager().SetTimer(m_ShakeTimer, Delegate, ShakeDuration, false);

			return;
		}

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		m_CollisionQueryParams.AddIgnoredActor(AttachedToActor);

		m_AxeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		m_RotatingMovementComponent->Activate();
	}
	else
	{
		m_ProjectileMovementComponent->Deactivate();
	}

	// Drop the carried hit actor.
	if (m_CarriedActor.IsValid())
	{
		m_CarriedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		IAxeDamageableInterface* const CarriedAxeDamageableActor{ Cast<IAxeDamageableInterface>(m_CarriedActor.Get()) };
		CarriedAxeDamageableActor->OnHit(m_HitResult, GetInstigator(), EAxeHitType::EAHT_Drop);
		m_CollisionQueryParams.AddIgnoredActor(AttachedToActor);
		m_CarriedActor.Reset();
	}

	m_InitialLocation = GetActorLocation();

	m_FlightInfo.m_TargetLocation = TargetLocation;
	m_FlightInfo.m_TargetRotation = TargetRotation;
	m_FlightInfo.m_bInterpingToTargetRotation = false;

	const double DistanceToTarget{ FVector::Distance(m_FlightInfo.m_TargetLocation, m_InitialLocation) };

	/*If the time calculated based on MinSpeed is less than FlightMaxDuration, it will be the chosen duration of the flight.*/
	m_FlightInfo.m_Duration = FMath::Min(DistanceToTarget / MinSpeed, FlightMaxDuration);

	/*The radius of the curved path the axe follows when flying to it's target.*/
	m_FlightInfo.m_PathCurveRadius = DistanceToTarget * 0.25;

	SetActorRotation(m_FlightInfo.m_TargetRotation);

	/*The axe will be at an angle when flying to it's target*/
	AddActorLocalRotation(FRotator{ 0.0, 0.0, 135.0 });

	GetWorldTimerManager().SetTimer(m_FlightInfo.m_FlightTimer, this, &ALeviathanAxe::OnFlightTimerCompleted, m_FlightInfo.m_Duration, false);

	const double DistSquaredToTarget{ FVector::DistSquared(m_FlightInfo.m_TargetLocation, m_InitialLocation) };

	if (DistSquaredToTarget <= m_FlightInfo.m_StartInterpRotDistSquared)
	{
		StartInterpToTargetRotation();
	}

	if (m_TrailEffect)
	{
		m_TrailEffect->Activate();
	}

	PlayAudioComponent(m_AxeSpinSound);
	m_bIsShaking = false;
	m_HitTypeRequest = EHitTypeRequest::EHTR_KnockBack;
}

void ALeviathanAxe::UpdateTargetRotationAndLocation(const FQuat& TargetRotation, const FVector& TargetLocation)
{
	m_FlightInfo.m_TargetRotation = TargetRotation;
	m_FlightInfo.m_TargetLocation = TargetLocation;
}

bool ALeviathanAxe::CheckForCollision()
{
	const FVector CurrentLocation{ GetActorLocation() };

	GetWorld()->LineTraceSingleByChannel(m_HitResult, m_LocationLastFrame, CurrentLocation, ECC_Visibility, m_CollisionQueryParams);

	m_LocationLastFrame = CurrentLocation;

	return m_HitResult.bBlockingHit;
}

void ALeviathanAxe::HandleCollision()
{
	OnHit.ExecuteIfBound(m_HitResult.GetActor(), false);

	m_CollisionQueryParams.ClearIgnoredActors();
	m_CollisionQueryParams.AddIgnoredActor(m_HitResult.GetActor());

	IAxeDamageableInterface* const AxeDamageableActor{ Cast<IAxeDamageableInterface>(m_HitResult.GetActor()) };

	{
		const bool bFlyingThroughNonAxeDamageableActor{ !AxeDamageableActor && (m_State == EAxeState::EAS_FlyingToTarget) };

		if (bFlyingThroughNonAxeDamageableActor) return;
	}

	const FVector PreImpactVelocity{ m_ProjectileMovementComponent->Velocity };

	/*Handles the case where the hit object derives from IAxeDamageableInterface.*/
	if (AxeDamageableActor)
	{
		HandleAxeDamageableCollision(AxeDamageableActor, PreImpactVelocity);
	}
	/*Handles the case where the hit object does not derive from IAxeDamageableInterface (most likely a static mesh from the environment.)*/
	else
	{
		DeactivateInAirComponents();

		/*If the axe is carrying an object, we need to store the objects location before it's modified by the axes transform changes.*/
		const FVector DraggedObjectPreImpactLocation{ m_CarriedActor.IsValid() ? m_CarriedActor->GetActorLocation() : FVector{}};

		/*Set new rotation.*/
		{
			const FVector Z{ (-m_HitResult.ImpactNormal).Cross(GetActorRightVector()) };
			const FRotator ImpactRotation{ FRotationMatrix::MakeFromYZ(GetActorRightVector(), Z).Rotator() };
			SetActorRotation(ImpactRotation);
			AddActorLocalRotation(FRotator{ FMath::FRandRange(-45.0, 3.0), 0.0, 0.0 });
		}

		/*Set new location.*/
		{
			ensureAlwaysMsgf(m_AxeMesh->GetSocketByName(m_ImpactSocketName), TEXT("Impact Socket not set! The socket name must be set in the blueprint and must match the socket name on the mesh itself."));
			const FVector ImpactSocketLocation{ m_AxeMesh->GetSocketLocation(m_ImpactSocketName) };
			SetActorLocation(GetActorLocation() + (m_HitResult.ImpactPoint - ImpactSocketLocation));
		}

		if (m_CarriedActor.Get())
		{
			/*Modify the rotation and location so the object appears to be pinned flat against the hit object.*/

			/*Align its rotation to face the same direction as the impact normal.*/
			const FVector Y{ (FVector::UpVector).Cross(m_HitResult.ImpactNormal) };
			const FRotator ImpactRotation{ FRotationMatrix::MakeFromXY(m_HitResult.ImpactNormal, Y).Rotator() };
			m_CarriedActor->SetActorRotation(ImpactRotation);
			
			/*Create an offset so the hit object isn't placed inside the pinned wall.*/
			m_CarriedActor->SetActorLocation(DraggedObjectPreImpactLocation);
			FVector LocationToImpactLocation{ DraggedObjectPreImpactLocation - m_HitResult.ImpactPoint };
			const double LocationToImpactLocationProjImpactNormal{ FVector::DotProduct(LocationToImpactLocation, m_HitResult.ImpactNormal) };
			FVector Offset{ -m_HitResult.ImpactNormal * LocationToImpactLocationProjImpactNormal };

			/*Add another offset that will move the hit actor along its right vector so it's at the center of the impact socket.*/
			const FVector LocationToImpactSocket{ m_AxeMesh->GetSocketLocation(m_ImpactSocketName) - m_CarriedActor->GetActorLocation() };
			const double LocationToImpactSocketProjObjectRight{ FVector::DotProduct(LocationToImpactSocket, m_CarriedActor->GetActorRightVector()) };
			Offset += LocationToImpactSocketProjObjectRight * m_CarriedActor->GetActorRightVector();
			
			m_CarriedActor->AddActorWorldOffset(Offset);
			
			OnHit.ExecuteIfBound(m_CarriedActor.Get(), true);

			if (IAxeDamageableInterface * CarriedAxeDamageableActor{ Cast<IAxeDamageableInterface>(m_CarriedActor.Get()) })
			{
				CarriedAxeDamageableActor->OnHit(m_HitResult, GetInstigator(), EAxeHitType::EAHT_PinToWall);
			}
		}

		AttachToComponent(m_HitResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);

		if (ensureAlwaysMsgf(m_AxeMesh, TEXT("Axe Mesh is null!")))
		{
			m_AxeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}

		m_AudioComponent->Stop();
		m_State = EAS_IdleAfterImpact;
	}

	if (ensureAlwaysMsgf(m_AxeImpactSound, TEXT("Axe Impact Sound is not set!")) && ensureAlwaysMsgf(m_SoundAttenuation, TEXT("Sound Attenuation is not set!")))
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_AxeImpactSound, GetActorLocation(), 0.6f, 1.0f, 0.0f, m_SoundAttenuation);
	}
}

void ALeviathanAxe::HandleAxeDamageableCollision(IAxeDamageableInterface* AxeDamageableActor, const FVector& PreImpactVelocity)
{
	if (!AxeDamageableActor->IsAlive()) return;

	// Search for a pinnable wall the we can pin the hit object to.
	bool bFoundPinnableWall{ false };

	{
		FHitResult PinnableWallHitResult{};
		const FVector TraceStart{ GetActorLocation() };
		const FVector TraceEnd{ TraceStart + PreImpactVelocity.GetSafeNormal() * m_PinnableWallSearchDistance };

		GetWorld()->LineTraceSingleByChannel(PinnableWallHitResult, TraceStart, TraceEnd, ECC_Visibility, m_CollisionQueryParams);

		if (m_CarriedActor.IsExplicitlyNull() && PinnableWallHitResult.bBlockingHit && Cast<APinnableWall>(PinnableWallHitResult.GetActor()))
		{
			bFoundPinnableWall = true;
		}
	}

	bool bLethalHit{ false };

	switch (m_HitTypeRequest)
	{
	case ALeviathanAxe::EHTR_KnockBack:
		bLethalHit = !AxeDamageableActor->OnHit(m_HitResult, GetInstigator(), EAxeHitType::EAHT_KnockBack);
		
		/*Go through the hit actor if it was killed or if the axe is flying.*/
		if (bLethalHit || (m_State == EAxeState::EAS_FlyingToTarget)) return;
		
		RicochetOffHitObject(PreImpactVelocity);
		break;

	case ALeviathanAxe::EHTR_AttachAndFreeze:
		if (bFoundPinnableWall)
		{
			AxeDamageableActor->OnHit(m_HitResult, GetInstigator(), EAxeHitType::EAHT_Carry);
			m_CarriedActor = Cast<AActor>(AxeDamageableActor);
			CarryHitActor(PreImpactVelocity);
		}
		else
		{
			bLethalHit = !AxeDamageableActor->OnHit(m_HitResult, GetInstigator(), EAxeHitType::EAHT_AttachToAndFreeze);
			
			if (bLethalHit) return;

			AttachToAxeDamageableActor(AxeDamageableActor);
		}

		break;

	default:
		break;
	}
}

void ALeviathanAxe::IdleAfterImpactTick()
{
	/*Shakes the axe mesh with Curve Floats by using it's values over time to animate the mesh.*/
	if (ensureAlwaysMsgf(m_PreFlightShakeCurve, TEXT("Pre-Flight Shake Curve is not set")) && m_bIsShaking)
	{
		const float ShakeRollOffset{ m_PreFlightShakeCurve->GetFloatValue(GetWorldTimerManager().GetTimerElapsed(m_ShakeTimer)) };
		m_AxeMesh->SetRelativeRotation(m_PreShakeMeshRelativeRotation + FRotator{ 0.0, 0.0, ShakeRollOffset });
	}
}

void ALeviathanAxe::PostLaunchTick()
{
	const FVector CurrentLocation{ GetActorLocation() };
	
	if (CheckForCollision())
	{
		HandleCollision();
	}
	else
	{
		/* Checks to see if the squared distance between the axe's current and initial location has passed m_DistanceSquaredToApplyGravity
		* and only if it shouldn't maintain a constant velocity
		*/
		const bool bGravityNotApplied{ !m_bMaintainConstantVelocity && !(m_ProjectileMovementComponent->ShouldApplyGravity()) };

		// If it did, apply gravity with the projectile movement component.
		if (bGravityNotApplied)
		{
			if (FVector::DistSquared(m_InitialLocation, GetActorLocation()) >= m_GravityTriggerDistanceSquared)
			{
				m_ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
			}
		}

		m_LocationLastFrame = CurrentLocation;
	}
}

void ALeviathanAxe::RicochetOffHitObject(const FVector& PreImpactVelocity)
{
	const FVector AxisOfRotation{ FVector::CrossProduct(PreImpactVelocity, FVector::UpVector).GetSafeNormal() };

	/*Rotate the velocity up by 70 degrees.*/
	static constexpr double RicochetVelocityDegreesOffset{ 70.0 };
	/*Decrease the velocity by 70 percent.*/
	static constexpr double RicochetVelocityMultiplier{ 0.3 };

	const FVector RicochetVelocity{ PreImpactVelocity.RotateAngleAxis(RicochetVelocityDegreesOffset, AxisOfRotation) * RicochetVelocityMultiplier };

	Launch(RicochetVelocity, m_HitTypeRequest);

	/*Make the axe fall quickly after ricocheting.*/
	static constexpr float RicochetGravityScale{ 3.0f };
	m_ProjectileMovementComponent->ProjectileGravityScale = RicochetGravityScale;
}

void ALeviathanAxe::AttachToAxeDamageableActor(IAxeDamageableInterface* AxeDamageableActor)
{
	if (AxeDamageableActor)
	{
		AxeDamageableActor->OnFreezeCompleted.BindUObject(this, &ALeviathanAxe::OnActorFreezeCompleted);

		DeactivateInAirComponents();

		if (USkeletalMeshComponent* const HitSkeletalMeshComponent{ AxeDamageableActor->GetSkeletalMeshComponent() })
		{
			const TArray<USkeletalMeshSocket*> Sockets{ HitSkeletalMeshComponent->GetSkinnedAsset()->GetActiveSocketList()};
			
			const USkeletalMeshSocket* TargetSocket{ nullptr };
			double MinDistanceSquared{ DOUBLE_BIG_NUMBER };
			
			for (int32 Idx{ 0 }; Idx < Sockets.Num(); ++Idx)
			{
				const double CurrentDistanceSquared{ FVector::DistSquared(m_HitResult.ImpactPoint, Sockets[Idx]->GetSocketLocation(HitSkeletalMeshComponent))};
			
				if (CurrentDistanceSquared < MinDistanceSquared)
				{
					TargetSocket = Sockets[Idx];
					MinDistanceSquared = CurrentDistanceSquared;
				}
			}

			if (TargetSocket)
			{
				const FRotator NewRotation{ TargetSocket->GetSocketTransform(HitSkeletalMeshComponent).Rotator() };
				SetActorRotation(NewRotation);

				ensureAlwaysMsgf(m_AxeMesh->GetSocketByName(m_ImpactSocketName), TEXT("Impact Socket not set! The socket name must be set in the blueprint and must match the socket name on the mesh itself."));
				const FVector ImpactSocketLocation{ m_AxeMesh->GetSocketLocation(m_ImpactSocketName) };
				SetActorLocation(GetActorLocation() + (TargetSocket->GetSocketLocation(HitSkeletalMeshComponent) - ImpactSocketLocation));
				AttachToComponent(HitSkeletalMeshComponent, FAttachmentTransformRules::KeepWorldTransform, TargetSocket->SocketName);
			}
			else
			{
				AttachToComponent(m_HitResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
		}

		m_AxeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		m_State = EAS_IdleAfterImpact;
	}
}

void ALeviathanAxe::CarryHitActor(const FVector& PreImpactVelocity)
{
	/*Now we need to adjust the velocity so the attached actor stays above ground.*/
	{
		const FVector AttachedActorDownVector{ -m_CarriedActor->GetActorUpVector() };
		const double AttachedActorDownProjVelocityDirection{ FVector::DotProduct(PreImpactVelocity.GetSafeNormal(), AttachedActorDownVector) };

		/*Velocity is pointing below the attached actor. Now we need to adjust it.*/
		if (AttachedActorDownProjVelocityDirection > 0.0)
		{
			m_ProjectileMovementComponent->Velocity = FVector::VectorPlaneProject(PreImpactVelocity, AttachedActorDownVector);
		}
	}

	/*Gravity should never be applied when the axe is carrying an object*/
	m_ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	m_bMaintainConstantVelocity = true;

	m_RotatingMovementComponent->Deactivate();
	ensureAlways(m_AxeMesh->DoesSocketExist(m_ImpactSocketName));

	/*Adjust the axes rotation so its aligned with the hit surface's normal.*/
	{
		const FVector AxeRightVector{ GetActorRightVector() };
		const FVector Z{ (-m_HitResult.ImpactNormal).Cross(AxeRightVector) };
		const FRotator ImpactRotation{ FRotationMatrix::MakeFromYZ(AxeRightVector, Z).Rotator() };
		SetActorRotation(ImpactRotation);

		m_CarriedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform, m_ImpactSocketName);
	}

	/* Adjust the carried objects rotation to the opposite direction of the axe.*/
	{
		m_CarriedActor->SetActorRotation(GetActorRotation());
		m_CarriedActor->AddActorLocalRotation(FRotator{ 0.0, 180.0, 0.0 });
	}

	/*We need to move the carried objects location more towards the impact socket to make it look like the axe 
	 *hit the mesh rather than the invisible root component (like a capsule component).
	*/
	{
		const FVector AxeRightVector{ GetActorRightVector() };
		const FVector CarriedObjectLocation{ m_CarriedActor->GetActorLocation() };
		const FVector CarriedObjectLocationToImpactSocket{ m_AxeMesh->GetSocketLocation(m_ImpactSocketName) - CarriedObjectLocation };
		const double CarriedObjectLocationToImpactSocketProjRight{ FVector::DotProduct(CarriedObjectLocationToImpactSocket, AxeRightVector) };
		m_CarriedActor->SetActorLocation(CarriedObjectLocation + (CarriedObjectLocationToImpactSocketProjRight * AxeRightVector));
	}

	m_State = EAS_PostLaunch;
}

void ALeviathanAxe::FlyToTargetTick()
{
	if (CheckForCollision())
	{
		HandleCollision();
	}

	/*When it's time, this interpolates the axes orientation to the target rotation so they will match once the flight is completed.*/
	if (m_FlightInfo.m_bInterpingToTargetRotation)
	{
		InterpToTargetRotation();
	}
	else
	{
		const double DistSquaredToTarget{ FVector::DistSquared(GetActorLocation(), m_FlightInfo.m_TargetLocation) };

		if (DistSquaredToTarget <= m_FlightInfo.m_StartInterpRotDistSquared)
		{
			StartInterpToTargetRotation();
		}
	}

	InterpToTargetLocation();
}

void ALeviathanAxe::InterpToTargetRotation()
{
	const float TimeElapsed{ GetWorldTimerManager().GetTimerElapsed(m_FlightInfo.m_InterpToRotTimer) };
	const double Alpha{ TimeElapsed / m_FlightInfo.m_InterpToTargetRotationDuration }; // The percentage of how close it is to the target rotation.
	const FQuat NewRotation{ FQuat::Slerp(m_FlightInfo.m_InitialRotation, m_FlightInfo.m_TargetRotation, Alpha) };
	SetActorRotation(NewRotation);
}

void ALeviathanAxe::InterpToTargetLocation()
{
	const float TimeElapsed{ GetWorldTimerManager().GetTimerElapsed(m_FlightInfo.m_FlightTimer) };
	// The percentage of how close it is to the target location.
	const double Alpha{ TimeElapsed / m_FlightInfo.m_Duration };

	FVector NewLocation{ FMath::Lerp(m_InitialLocation, m_FlightInfo.m_TargetLocation, Alpha) };
	// Equation used to calculate the curved path of the axe's flight every frame.
	const FVector PathCurveOffset{ m_FlightInfo.m_TargetRotation.GetRightVector() * FMath::Sin(UE_PI * Alpha) * m_FlightInfo.m_PathCurveRadius };

	NewLocation += PathCurveOffset;

	SetActorLocation(NewLocation);
}

void ALeviathanAxe::StartInterpToTargetRotation()
{
	if (!m_FlightInfo.m_bInterpingToTargetRotation)
	{
		m_RotatingMovementComponent->Deactivate();
		m_FlightInfo.m_InitialRotation = GetActorQuat();
		m_FlightInfo.m_bInterpingToTargetRotation = true;
		m_FlightInfo.m_InterpToTargetRotationDuration = GetWorldTimerManager().GetTimerRemaining(m_FlightInfo.m_FlightTimer);
		
		GetWorldTimerManager().SetTimer(m_FlightInfo.m_InterpToRotTimer, m_FlightInfo.m_InterpToTargetRotationDuration, false);
	}
}

void ALeviathanAxe::OnFlightTimerCompleted()
{
	m_AxeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetActorRotation(m_FlightInfo.m_TargetRotation);
	SetActorLocation(m_FlightInfo.m_TargetLocation);

	if (m_TrailEffect)
	{
		m_TrailEffect->Deactivate();
	}

	m_AudioComponent->Stop();

	if (ensureAlwaysMsgf(m_AxeImpactSound, TEXT("Axe Impact Sound is not set!")))
	{
		PlayAudioComponent(m_AxeImpactSound);
	}

	m_CollisionQueryParams.ClearIgnoredActors();
	OnFlightCompleted.ExecuteIfBound();
	m_State = EAS_Idle;
}

void ALeviathanAxe::OnActorFreezeCompleted()
{
	if (m_State == EAS_IdleAfterImpact && GetAttachParentActor())
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		m_AxeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		m_ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
		m_ProjectileMovementComponent->Velocity = FVector::ZeroVector;
		m_ProjectileMovementComponent->Activate();

		m_InitialLocation = GetActorLocation();
		m_LocationLastFrame = m_InitialLocation;

		m_State = EAS_PostLaunch;
	}
}

void ALeviathanAxe::DeactivateInAirComponents()
{
	m_RotatingMovementComponent->Deactivate();
	m_ProjectileMovementComponent->Deactivate();

	if (m_TrailEffect)
	{
		m_TrailEffect->Deactivate();
	}

	m_AudioComponent->Stop();
}

void ALeviathanAxe::PlayAudioComponent(USoundBase* Sound)
{
	if (Sound)
	{
		m_AudioComponent->SetSound(Sound);
	}

	m_AudioComponent->Play();
}
