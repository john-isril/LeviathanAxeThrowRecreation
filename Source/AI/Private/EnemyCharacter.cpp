// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include <AIController.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <Perception/PawnSensingComponent.h>
#include <Components/CapsuleComponent.h>
#include "EnemySword.h"
#include "EnemyHealthComponent.h"
#include <BrainComponent.h>
#include <Kismet/GameplayStatics.h>

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator{ 0.0, 200.0, 0.0 };

	m_PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
	m_PawnSensingComponent->SetPeripheralVisionAngle(45.0f);
	m_PawnSensingComponent->SightRadius = 1000.0f;

	m_EnemyHealthComponent = CreateDefaultSubobject<UEnemyHealthComponent>(TEXT("EnemyHealthComponent"));

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	GetCharacterMovement()->MaxWalkSpeed = m_WalkSpeed;
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	m_PawnSensingComponent->OnSeePawn.AddUniqueDynamic(this, &AEnemyCharacter::OnSeePawn);

	m_TargetActor = nullptr;
	
	m_AttackMaxRadiusSquared = m_AttackMaxRadius * m_AttackMaxRadius;

	m_EnemyState = EEnemyState::EES_Patrolling;
	
	m_PreStunnedEnemyState = EEnemyState::EES_None;

	m_bIsTurningToTarget = false;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_AIController = CastChecked<AAIController>(GetController());
	
	if (UBlackboardComponent* const BBComp{ m_AIController->GetBlackboardComponent() })
	{
		BBComp->SetValueAsBool(m_CanMoveWhilePatrollingBBKeyName, m_bCanMoveWhilePatrolling);
		BBComp->SetValueAsFloat(m_AttackMaxRadiusBBKeyName, m_AttackMaxRadius);
	}

	s_AllEnemies.Add(this);
	s_AttackingEnemies.Empty();
	
	if (ensureAlwaysMsgf(m_EnemySwordClass, TEXT("Enemy Sword Class is not set!")))
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		m_Sword = GetWorld()->SpawnActor<AEnemySword>(m_EnemySwordClass, Params);

		ensureAlwaysMsgf(GetMesh()->GetSocketByName(m_WeaponSocketName), TEXT("Weapon socket not found on skeletal mesh!"));
		m_Sword->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, m_WeaponSocketName);
	}

	GetWorldTimerManager().SetTimer(m_GrowlDelayTimer, this, &AEnemyCharacter::OnGrowlDelayTimerCompleted, FMath::RandRange(m_GrowlDelayMinDuration, m_GrowlDelayMaxDuration), false);
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_EnemyState == EEnemyState::EES_Dead) return;

	/*Turning in place logic.*/
	{
		FVector DirectionToTarget{};

		CheckTurnToTarget(DirectionToTarget);

		if (m_bIsTurningToTarget)
		{
			TurnToTarget(DirectionToTarget, DeltaTime);
		}
	}
}

void AEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GetWorldTimerManager().ClearTimer(m_GrowlDelayTimer);
	GetWorldTimerManager().ClearTimer(m_FreezeTimer);

	if (s_AllEnemies.Contains(this)) s_AllEnemies.Remove(this);
	
	if (s_AttackingEnemies.Contains(this)) s_AttackingEnemies.Remove(this);
}

float AEnemyCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	m_EnemyHealthComponent->ReceiveDamage(Damage);

	return Damage;
}

bool AEnemyCharacter::OnHit(const FHitResult& HitResult, AActor* Causer, EAxeHitType AxeHitType)
{
	if (ensureAlwaysMsgf(m_HitSound, TEXT("Hit Sound is not set!")) && ensureAlwaysMsgf(m_SoundAttenuation, TEXT("Hit Sound Attenuation is not set!")))
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_HitSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, m_SoundAttenuation);
	}

	if (ensureAlwaysMsgf(m_BloodSplatterParticle, TEXT("Blood Splatter Particle is not set!")))
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, m_BloodSplatterParticle, HitResult.ImpactPoint);
	}

	/*Death needs to be delayed when being carried or being pinned.*/
	const bool bStartDeathReaction = m_EnemyHealthComponent->IsDead() &&
									(AxeHitType != EAxeHitType::EAHT_Carry) &&
									(AxeHitType != EAxeHitType::EAHT_PinToWall) &&
									(AxeHitType != EAxeHitType::EAHT_Drop);

	if (bStartDeathReaction)
	{
		OnDeathStarted();
		return false;
	}

	const FVector HitDirection{ (HitResult.TraceEnd - HitResult.TraceStart).GetSafeNormal() };

	switch (AxeHitType)
	{
	case EAxeHitType::EAHT_KnockBack:
		GetKnockedBack(HitDirection);

		break;

	case EAxeHitType::EAHT_AttachToAndFreeze:
		GetKnockedBack(HitDirection);
		Freeze();

		break;

	case EAxeHitType::EAHT_DetachFrom:
		Unfreeze();

		break;

	case EAxeHitType::EAHT_Carry:
		StopAnimMontage();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EnterStunnedState();
		OnCarried.ExecuteIfBound();
		
		break;

	case EAxeHitType::EAHT_PinToWall:
		m_Sword->DropFromParent();
		OnPinnedToWall.ExecuteIfBound();
		
		if (m_EnemyState != EEnemyState::EES_Stunned)
		{
			EnterStunnedState();
		}

		break;

	case EAxeHitType::EAHT_Drop:
		/*If the EnemyCharacter is dead before being dropped, we can complete the death logic (applying ragdoll physics, etc.).*/
		if (m_EnemyHealthComponent->IsDead())
		{
			OnDeathCompleted(true);
			
			return false;
		}

		/*If dropped, the EnemyCharacter will get back on it's feet.*/
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RecoverFromStunnedState();
		OnDropped.ExecuteIfBound();

		break;
	
	default:
		break;
	}

	if (m_PreStunnedEnemyState == EEnemyState::EES_Patrolling)
	{
		AlertAllEnemies(Causer, true);
	}

	return true;
}

void AEnemyCharacter::Freeze()
{
	const FName CurrentSectionName{ GetMesh()->GetAnimInstance()->Montage_GetCurrentSection() };
	const int32 CurrentSectionIdx{ m_HitReactMontageInfo.m_Montage->GetSectionIndex(CurrentSectionName) };
	const float FreezeDelayTime{ m_HitReactMontageInfo.m_Montage->GetSectionLength(CurrentSectionIdx) * 0.5f };

	/*Begin the delay before freezing the enemy.*/
	GetWorldTimerManager().SetTimer(m_FreezeTimer, [this]() {
		if (m_EnemyState == EEnemyState::EES_Stunned)
		{
			GetMesh()->GetAnimInstance()->Montage_Pause(m_HitReactMontageInfo.m_Montage);
			/*Set the timer that unfreezes the enemy for m_FreezeMaxDuration seconds.*/
			GetWorldTimerManager().SetTimer(m_FreezeTimer, this, &AEnemyCharacter::Unfreeze, m_FreezeMaxDuration, false);
		}
	}, FreezeDelayTime, false);

	GetMesh()->SetScalarParameterValueOnMaterials(m_FreezeTimeMaterialParameterName, GetWorld()->TimeSeconds);

	if (m_EnemyState != EEnemyState::EES_Stunned)
	{
		EnterStunnedState();
	}
}

void AEnemyCharacter::Unfreeze()
{
	if (m_EnemyState == EEnemyState::EES_Stunned)
	{
		GetWorldTimerManager().ClearTimer(m_FreezeTimer);
		GetMesh()->GetAnimInstance()->Montage_Resume(m_HitReactMontageInfo.m_Montage);
		GetMesh()->SetScalarParameterValueOnMaterials(m_FreezeTimeMaterialParameterName, 0.0f);
		RecoverFromStunnedState();

		OnFreezeCompleted.ExecuteIfBound();
	}
}

USkeletalMeshComponent* AEnemyCharacter::GetSkeletalMeshComponent() const
{
	return GetMesh();
}

bool AEnemyCharacter::IsAlive() const
{
	return m_EnemyState != EEnemyState::EES_Dead;
}

void AEnemyCharacter::StartPursuing()
{
	StopAnimMontage();
	m_EnemyState = EEnemyState::EES_PursuingTargetActor;
	m_AIController->GetBlackboardComponent()->SetValueAsEnum(m_EnemyStateBBKeyName, uint8(m_EnemyState));
	GetCharacterMovement()->MaxWalkSpeed = m_RunSpeed;
	GetWorldTimerManager().ClearTimer(m_GrowlDelayTimer);
}

void AEnemyCharacter::StartAttacking()
{
	m_EnemyState = EEnemyState::EES_AttackingTargetActor;
	m_AIController->GetBlackboardComponent()->SetValueAsEnum(m_EnemyStateBBKeyName, uint8(m_EnemyState));
	m_bIsTurningToTarget = false;
	GetCharacterMovement()->MaxWalkSpeed = m_AttackingRunSpeed;

	if (s_AttackingEnemies.Find(this)) return;

	/*If this enemy instance wants to start attacking but the number of attacking enemies is maxed out, switch out an already attacking enemy.*/	
	if (AttackingEnemiesAtMaxCapacity())
	{
		AEnemyCharacter* const SwitchedOutFighter{ *(s_AttackingEnemies.begin()) };
		s_AttackingEnemies.Remove(SwitchedOutFighter);

		if (SwitchedOutFighter->m_EnemyState == EEnemyState::EES_Stunned)
		{
			SwitchedOutFighter->m_PreStunnedEnemyState = EEnemyState::EES_Spectating;
		}
		else
		{
			SwitchedOutFighter->GetCharacterMovement()->MaxWalkSpeed = m_RunSpeed;
			SwitchedOutFighter->m_EnemyState = EEnemyState::EES_Spectating;
			SwitchedOutFighter->m_AIController->GetBlackboardComponent()->SetValueAsEnum(SwitchedOutFighter->m_EnemyStateBBKeyName, uint8(SwitchedOutFighter->m_EnemyState));
		}

		s_AttackingEnemies.Add(this);
		StopAnimMontage(m_ScreamMontageInfo.m_Montage);
	}
	else
	{
		s_AttackingEnemies.Add(this);

		/*If the number of fighting enemies is at max capacity, alert all non-fighters to start spectating.*/
		if (AttackingEnemiesAtMaxCapacity())
		{
			for (AEnemyCharacter* const Enemy : s_AllEnemies)
			{
				if (Enemy->m_EnemyState == EEnemyState::EES_PursuingTargetActor)
				{
					Enemy->m_EnemyState = EEnemyState::EES_Spectating;
					Enemy->m_AIController->GetBlackboardComponent()->SetValueAsEnum(Enemy->m_EnemyStateBBKeyName, uint8(Enemy->m_EnemyState));
				}
			}
		}
	}
}

void AEnemyCharacter::Scream()
{
	if ((GetCurrentMontage() != m_ScreamMontageInfo.m_Montage) && ensureAlwaysMsgf(m_ScreamMontageInfo.m_Montage, TEXT("Montage in Scream Montage is not set!")))
	{
		PlayAnimMontage(m_ScreamMontageInfo.m_Montage, 1.0f, m_ScreamMontageInfo.GetRandomSection());
	}

	if (ensureAlwaysMsgf(m_ScreamSound, TEXT("Scream Sound is not set!")) && ensureAlwaysMsgf(m_LoudSoundAttenuation, TEXT("Loud Sound Attenuation is not set!")))
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_ScreamSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, m_LoudSoundAttenuation);
	}
}

void AEnemyCharacter::Attack()
{
	if ((GetCurrentMontage() != m_AttackMontageInfo.m_Montage) && ensureAlwaysMsgf(m_AttackMontageInfo.m_Montage, TEXT("Montage in Attack Montage is not set!")))
	{
		PlayAnimMontage(m_AttackMontageInfo.m_Montage, 1.0f, m_AttackMontageInfo.GetRandomSection());
	}

	if (ensureAlwaysMsgf(m_ScreamSound, TEXT("Attack Sound is not set!")) && ensureAlwaysMsgf(m_SoundAttenuation, TEXT("Sound Attenuation is not set!")))
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_AttackSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, m_SoundAttenuation);
	}

}

void AEnemyCharacter::OnScreamMontageCompleted()
{
	if (m_EnemyState == EEnemyState::EES_None)
	{
		StartPursuing();
	}

	OnActionCompleted.ExecuteIfBound();
}

void AEnemyCharacter::ActivateWeaponDamage(bool Activate)
{
	m_Sword->EnableDamage(Activate);
}

void AEnemyCharacter::OnAttackMontageCompleted()
{
	OnActionCompleted.ExecuteIfBound();
}

void AEnemyCharacter::OnHitReactMontageCompleted()
{
	RecoverFromStunnedState();
}

void AEnemyCharacter::OnDeathCompleted(bool bRagdollDeath)
{
	if (!GetAttachParentActor())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		if (bRagdollDeath)
		{
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName(FName{ "Ragdoll" });
		}

		SetLifeSpan(m_DeathDuration);
		m_Sword->SetLifeSpan(m_DeathDuration);
		OnDeathDelegate.Broadcast();
	}
}

void AEnemyCharacter::OnGrowlDelayTimerCompleted()
{
	if (ensureAlwaysMsgf(m_ScreamSound, TEXT("Growl Sound is not set!")) && ensureAlwaysMsgf(m_SoundAttenuation, TEXT("Sound Attenuation is not set!")))
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_GrowlSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, m_SoundAttenuation);
		GetWorldTimerManager().SetTimer(m_GrowlDelayTimer, this, &AEnemyCharacter::OnGrowlDelayTimerCompleted, FMath::RandRange(m_GrowlDelayMinDuration, m_GrowlDelayMaxDuration), false);
	}
}

void AEnemyCharacter::OnSeePawn(APawn* Pawn)
{
	if ((m_EnemyState == EEnemyState::EES_Patrolling) && !Cast<AEnemyCharacter>(Pawn))
	{
		AlertAllEnemies(Pawn, false);
	}
}

void AEnemyCharacter::AlertAllEnemies(AActor* TargetActor, bool bIgnoreSelf)
{
	checkf(TargetActor, TEXT("TargetActor is null!"));

	for (AEnemyCharacter* Enemy : s_AllEnemies)
	{
		Enemy->m_PawnSensingComponent->Deactivate();
		Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->m_RunSpeed;
		Enemy->m_TargetActor = TargetActor;
		Enemy->m_EnemyHealthComponent->SetTargetActor(TargetActor);
		Enemy->m_AIController->GetBlackboardComponent()->SetValueAsObject(m_TargetActorBBKeyName, TargetActor);
		
		GetWorldTimerManager().ClearTimer(m_GrowlDelayTimer);

		if (bIgnoreSelf && (Enemy == this)) continue;
		
		Enemy->m_EnemyState = EEnemyState::EES_None;
		Enemy->m_AIController->GetBlackboardComponent()->SetValueAsEnum(m_EnemyStateBBKeyName, uint8(Enemy->m_EnemyState));
		Enemy->Scream();
	}
}

void AEnemyCharacter::CheckTurnToTarget(FVector& DirectionToTarget)
{
	if (!m_TargetActor.IsValid())
	{
		m_bIsTurningToTarget = false;
		return;
	}

	if ((m_EnemyState != EEnemyState::EES_AttackingTargetActor) && (m_EnemyState != EEnemyState::EES_Spectating))
	{
		m_bIsTurningToTarget = false;
		return;
	}

	const double SpeedSquared{ GetCharacterMovement()->Velocity.SizeSquared2D() };

	if (!FMath::IsNearlyZero(SpeedSquared)) {

		m_bIsTurningToTarget = false;

		return;
	}

	DirectionToTarget = (m_TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	DirectionToTarget.Z = 0.0f;

	const double CosAngleBetween{ FVector::DotProduct(GetActorForwardVector(), DirectionToTarget) };

	if (m_bIsTurningToTarget)
	{
		if (FMath::IsNearlyEqual(CosAngleBetween, 1.0, 0.02))
		{
			m_bIsTurningToTarget = false;
		}
	}
	else
	{
		m_bIsTurningToTarget = CosAngleBetween < (m_PawnSensingComponent->GetPeripheralVisionCosine());
	}
}

void AEnemyCharacter::TurnToTarget(const FVector& TargetDirection, float DeltaTime)
{
	const FRotator TargetRotation{ FRotationMatrix::MakeFromXZ(TargetDirection, FVector::UpVector).Rotator() };

	SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), TargetRotation, DeltaTime, 300.0f));
}

void AEnemyCharacter::GetKnockedBack(const FVector& HitDirection)
{
	if (!ensureAlwaysMsgf(m_HitReactMontageInfo.m_Montage, TEXT("Montage in Hit React Montage is not set!"))) return;

	/*Plays the hit react montage and plays the section name according to the direction of the hit relative to the enemy.*/
	const double HitDirectionProjEnemyFwd{ FVector::DotProduct(HitDirection, GetActorForwardVector()) };
	FName HitReactSectionName{ "" };

	static constexpr double Cos45Degrees{ 0.71 };

	if (HitDirectionProjEnemyFwd >= Cos45Degrees)
	{
		HitReactSectionName = m_HitReactMontageInfo.m_BackSectionName;
	}
	else if (HitDirectionProjEnemyFwd <= -Cos45Degrees) // 45 degrees from the back vector, so not it's not -45 degrees. It's the negation of cosine of 45 degrees. 
	{
		HitReactSectionName = m_HitReactMontageInfo.m_FrontSectionName;
	}
	else
	{
		const double HitDirectionProjEnemyRight{ FVector::DotProduct(HitDirection, GetActorRightVector()) };

		if (HitDirectionProjEnemyRight > 0.0)
		{
			HitReactSectionName = m_HitReactMontageInfo.m_LeftSectionName;
		}
		else
		{
			HitReactSectionName = m_HitReactMontageInfo.m_RightSectionName;
		}
	}

	PlayAnimMontage(m_HitReactMontageInfo.m_Montage, 1.0f, HitReactSectionName);

	if (m_EnemyState != EEnemyState::EES_Stunned)
	{
		EnterStunnedState();
	}
}

void AEnemyCharacter::EnterStunnedState()
{
	/*Keep track of the state before the enemy was stunned in order to return back to it later on.*/
	m_PreStunnedEnemyState = m_EnemyState;
	m_EnemyState = EEnemyState::EES_Stunned;

	if (UBlackboardComponent* const BBComp{ m_AIController->GetBlackboardComponent() })
	{
		BBComp->SetValueAsEnum(m_EnemyStateBBKeyName, uint8(m_EnemyState));
	}

	if (m_PreStunnedEnemyState == EEnemyState::EES_AttackingTargetActor)
	{
		m_Sword->EnableDamage(false);
	}
}

void AEnemyCharacter::RecoverFromStunnedState()
{
	if (m_EnemyState == EEnemyState::EES_Stunned)
	{
		m_EnemyState = (m_PreStunnedEnemyState == EEnemyState::EES_None || (m_PreStunnedEnemyState == EEnemyState::EES_Patrolling)) ? EEnemyState::EES_PursuingTargetActor : m_PreStunnedEnemyState;
		m_PreStunnedEnemyState = EEnemyState::EES_None;
		m_AIController->GetBlackboardComponent()->SetValueAsEnum(m_EnemyStateBBKeyName, uint8(m_EnemyState));
		GetCharacterMovement()->MaxWalkSpeed = (m_EnemyState == EEnemyState::EES_AttackingTargetActor) ? m_AttackingRunSpeed : m_RunSpeed;
	}
}

void AEnemyCharacter::OnDeathStarted()
{
	m_Sword->DropFromParent();

	s_AllEnemies.Remove(this);

	if (s_AttackingEnemies.Find(this))
	{
		s_AttackingEnemies.Remove(this);

		/*Iterate through the set of enemies to find one to take this dieing enemies position.*/
		for (AEnemyCharacter* const Enemy : s_AllEnemies)
		{
			if (Enemy->m_EnemyState == EEnemyState::EES_Spectating)
			{
				Enemy->StartPursuing();
				break;
			}
		}
	}

	m_AIController->GetBrainComponent()->StopLogic(FString{ "Died" });
	
	if (GetWorldTimerManager().IsTimerActive(m_FreezeTimer))
	{
		GetMesh()->SetScalarParameterValueOnMaterials(m_FreezeTimeMaterialParameterName, 0.0f);
	}

	if (GetAttachParentActor()) return;
	
	if (ensureAlwaysMsgf(m_DeathMontageInfo.m_Montage, TEXT("Montage in Death Montage is not set!")))
	{
		PlayAnimMontage(m_DeathMontageInfo.m_Montage, 1.0f, m_DeathMontageInfo.GetRandomSection());
	}

	m_EnemyState = EEnemyState::EES_Dead;
}