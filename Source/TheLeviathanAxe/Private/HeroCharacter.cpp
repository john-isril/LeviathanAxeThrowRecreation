#include "HeroCharacter.h"
#include "HeroPlayerController.h"
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>
#include <InputActionValue.h>
#include "HeroAnimInstance.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include "AxeCombatComponent.h"
#include "HealthComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Main_GameMode.h"

AHeroCharacter::AHeroCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate the character with the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->MaxWalkSpeed = m_MaxWalkSpeed;
	GetCharacterMovement()->RotationRate = FRotator{ 0.0, 400.0, 0.0 };

	m_CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	m_CameraBoom->SetupAttachment(GetRootComponent());
	m_CameraBoom->TargetArmLength = 150.0f;

	m_FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	m_FollowCamera->SetupAttachment(m_CameraBoom, USpringArmComponent::SocketName);
	m_FollowCamera->bUsePawnControlRotation = false;

	m_AxeCombatComponent = CreateDefaultSubobject<UAxeCombatComponent>(TEXT("Axe Combat Component"));
	
	m_HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
}

void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* const EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ensureAlwaysMsgf(m_Move_IA, TEXT("Move Input Action is not set!")))
		{
			EnhancedInput->BindAction(m_Move_IA, ETriggerEvent::Started, this, &AHeroCharacter::OnMoveStarted);
			EnhancedInput->BindAction(m_Move_IA, ETriggerEvent::Triggered, this, &AHeroCharacter::OnMoveTriggered);
			EnhancedInput->BindAction(m_Move_IA, ETriggerEvent::Completed, this, &AHeroCharacter::OnMoveCompleted);
		}

		if (ensureAlwaysMsgf(m_Look_IA, TEXT("Look Input Action is not set!")))
		{
			EnhancedInput->BindAction(m_Look_IA, ETriggerEvent::Triggered, this, &AHeroCharacter::OnLookTriggered);
		}

		if (ensureAlwaysMsgf(m_Sprint_IA, TEXT("Sprint Input Action is not set!")))
		{
			EnhancedInput->BindAction(m_Sprint_IA, ETriggerEvent::Started, this, &AHeroCharacter::OnSprintStarted);
			EnhancedInput->BindAction(m_Sprint_IA, ETriggerEvent::Completed, this, &AHeroCharacter::OnSprintCompleted);
		}

		if (ensureAlwaysMsgf(m_Aim_IA, TEXT("Aim Input Action is not set!")))
		{
			EnhancedInput->BindAction(m_Aim_IA, ETriggerEvent::Started, this, &AHeroCharacter::OnAimStarted);
			EnhancedInput->BindAction(m_Aim_IA, ETriggerEvent::Completed, this, &AHeroCharacter::OnAimCompleted);
		}

		if (ensureAlwaysMsgf(m_HeavyAttack_IA, TEXT("Heavy Attack Input Action is not set!")))
		{
			EnhancedInput->BindAction(m_HeavyAttack_IA, ETriggerEvent::Triggered, this, &AHeroCharacter::OnHeavyAttackTriggered);
		}

		if (ensureAlwaysMsgf(m_LightAttack_IA, TEXT("Light Attack Input Action is not set!")))
		{
			EnhancedInput->BindAction(m_LightAttack_IA, ETriggerEvent::Triggered, this, &AHeroCharacter::OnLightAttackTriggered);
		}

		if (ensureAlwaysMsgf(m_RecallAxe_IA, TEXT("Recall Axe Input Action is not set!")))
		{
			EnhancedInput->BindAction(m_RecallAxe_IA, ETriggerEvent::Triggered, this, &AHeroCharacter::OnRecallAxeTriggered);
		}

		if (ensureAlwaysMsgf(m_Pause_IA, TEXT("Pause Input Action is not set!")))
		{
			EnhancedInput->BindAction(m_Pause_IA, ETriggerEvent::Triggered, this, &AHeroCharacter::OnPause);
		}
	}
}

void AHeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	m_CameraBoom->bUsePawnControlRotation = true;
	m_CameraBoom->bInheritPitch = true;
	m_CameraBoom->bInheritYaw = true;
	m_CameraBoom->bInheritRoll = false;

	m_AnimInstance = CastChecked<UHeroAnimInstance>(GetMesh()->GetAnimInstance());

	SetStateIdle();

	/* Get notified for when the axe is thrown.*/
	m_AxeCombatComponent->OnAxeThrown.BindUObject(this, &AHeroCharacter::OnAxeThrown);
	
	/* Get notified for when the axe is recalled back to the characters hand.*/
	m_AxeCombatComponent->OnAxeRecallCompleted.BindUObject(this, &AHeroCharacter::OnAxeRecallCompleted);
	
	/* Get notified for when the character has zero health.*/
	m_HealthComponent->OnDeath.BindUObject(this, &AHeroCharacter::OnDeath);

	m_HeroFlags = EHeroFlags::EHF_Equipped;
}

void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// If CastChecked fails, it's most likely because the controller wasn't set in the game mode.
	m_HeroPlayerController = CastChecked<AHeroPlayerController>(GetController());

	m_EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(m_HeroPlayerController->GetLocalPlayer());
	checkf(m_EnhancedInputLocalPlayerSubsystem.Get(), TEXT("Unable to retrieve the Enhanced Input Local Player Subsystem!"));

	ensureAlwaysMsgf(m_Hero_IMC, TEXT("Hero Input Mapping Context is not set!"));
	m_EnhancedInputLocalPlayerSubsystem->AddMappingContext(m_Hero_IMC, 0);
}

void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/*
* The axe throw montage will call a notify function on the UHeavyAxeThrowAnimNotify class.
*/
void AHeroCharacter::PLayAxeThrowAnimMontage(bool bHeavy)
{
	if (IsAnyMontagePlaying()) return;

	if (ensureAlwaysMsgf(m_AxeThrowMontage, TEXT("Axe Throw montage is not set!")))
	{
		PlayAnimMontage(m_AxeThrowMontage, 1.0f, bHeavy ? m_HeavyAxeThrowAnimMontageSectionName : m_LightAxeThrowAnimMontageSectionName);
	}
}

bool AHeroCharacter::IsAnyMontagePlaying() const
{
	if (!m_AnimInstance) return false;

	return m_AnimInstance->IsAnyMontagePlaying();
}

bool AHeroCharacter::IsAiming() const
{
	return (m_HeroState == EHeroState::EHS_AimingInPlace) || (m_HeroState == EHeroState::EHS_AimWalking);
}

void AHeroCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void AHeroCharacter::OnMoveStarted(const FInputActionValue& Value)
{
	switch (m_HeroState)
	{
	case EHeroState::EHS_Idle:
		if (SprintButtonDown())
		{
			SetStateSprinting();
		}
		else
		{
			SetStateWalking();
		}

		break;

	case EHeroState::EHS_AimingInPlace:
		if (SprintButtonDown())
		{
			m_AxeCombatComponent->StopAiming();
			SetStateSprinting();
		}
		else
		{
			SetStateAimWalking();
		}

		break;
	
	default:
		break;
	}
}

void AHeroCharacter::OnMoveTriggered(const FInputActionValue& Value)
{
	if (Stunned()) return;

	const FVector2D ValueVector2D{ Value.Get<FVector2D>() };
	const FRotator ControllerYawRotation{ 0.0, GetControlRotation().Yaw, 0.0 };
	const FRotationMatrix ControllerRotationMatrix{ ControllerYawRotation };
	const FVector HeroFwd{ ControllerRotationMatrix.GetUnitAxis(EAxis::X) };
	const FVector HeroRight{ ControllerRotationMatrix.GetUnitAxis(EAxis::Y) };

	AddMovementInput(HeroFwd, ValueVector2D.X);
	AddMovementInput(HeroRight, ValueVector2D.Y);
}

void AHeroCharacter::OnMoveCompleted(const FInputActionValue& Value)
{
	switch (m_HeroState)
	{
	case EHeroState::EHS_AimWalking:
		SetStateAimingInPlace();
		break;
	
	case EHeroState::EHS_Walking:
		SetStateIdle();
		break;
	
	case EHeroState::EHS_Sprinting:
		SetStateIdle();
		break;
	
	default:
		break;
	}
}

void AHeroCharacter::OnLookTriggered(const FInputActionValue& Value)
{
	const FVector2D InputAxes{ Value.Get<FVector2D>() };

	AddControllerYawInput(InputAxes.X);
	AddControllerPitchInput(InputAxes.Y);
}

void AHeroCharacter::OnSprintStarted(const FInputActionValue& Value)
{
	m_HeroInputFlags |= EHeroInputFlags::EHIF_SprintButtonDown;

	switch (m_HeroState)
	{
	case EHeroState::EHS_Idle:
		break;

	case EHeroState::EHS_AimingInPlace:
		break;

	case EHeroState::EHS_AimWalking:
		m_AxeCombatComponent->StopAiming();
		SetStateSprinting();
		break;

	case EHeroState::EHS_Walking:
		SetStateSprinting();
		break;
	
	default:
		break;
	}
}

void AHeroCharacter::OnSprintCompleted(const FInputActionValue& Value)
{
	m_HeroInputFlags &= ~EHeroInputFlags::EHIF_SprintButtonDown;
	
	switch (m_HeroState)
	{
	case EHeroState::EHS_Sprinting:
		if (IsEquipped() && AimButtonDown())
		{
			m_AxeCombatComponent->Aim();
			SetStateAimWalking();
		}
		else
		{
			SetStateWalking();
		}

		break;

	default:
		break;
	}
}

void AHeroCharacter::OnAimStarted(const FInputActionValue& Value)
{
	m_HeroInputFlags |= EHeroInputFlags::EHIF_AimButtonDown;

	m_AxeCombatComponent->Aim();

	if (!IsEquipped()) return;

	switch (m_HeroState)
	{
	case EHeroState::EHS_Idle:
		SetStateAimingInPlace();
		break;
	
	case EHeroState::EHS_Walking:
		SetStateAimWalking();
		break;
	
	case EHeroState::EHS_Sprinting:
		SetStateAimWalking();
		break;
	
	default:
		break;
	}

}

void AHeroCharacter::OnAimCompleted(const FInputActionValue& Value)
{
	m_HeroInputFlags &= ~EHeroInputFlags::EHIF_AimButtonDown;
	
	m_AxeCombatComponent->StopAiming();

	if (!IsEquipped()) return;
	
	switch (m_HeroState)
	{
	case EHeroState::EHS_Idle:
		break;
	
	case EHeroState::EHS_AimingInPlace:
		SetStateIdle();
		break;
	
	case EHeroState::EHS_AimWalking:
		if (SprintButtonDown())
		{
			SetStateSprinting();
		}
		else
		{
			SetStateWalking();
		}
		
		break;
	
	default:
		break;
	}
}

void AHeroCharacter::OnHeavyAttackTriggered(const FInputActionValue& Value)
{
	if (IsEquipped() && !Stunned())
	{
		m_AxeCombatComponent->HeavyAttack();
	}
}

void AHeroCharacter::OnLightAttackTriggered(const FInputActionValue& Value)
{
	if (IsEquipped() && !Stunned())
	{
		m_AxeCombatComponent->LightAttack();
	}
}

void AHeroCharacter::OnRecallAxeTriggered(const FInputActionValue& Value)
{
	if (!IsRecallingAxe() && !IsEquipped() && !Stunned())
	{
		m_HeroFlags |= EHeroFlags::EHF_RecallingAxe;
		m_AxeCombatComponent->RecallAxe();
	}
}

void AHeroCharacter::OnPause(const FInputActionValue& Value)
{
	m_HeroPlayerController->TogglePauseMenu();
}

void AHeroCharacter::SetStateIdle()
{
	m_HeroState = EHeroState::EHS_Idle;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void AHeroCharacter::SetStateWalking()
{
	m_HeroState = EHeroState::EHS_Walking;
	GetCharacterMovement()->MaxWalkSpeed = m_MaxWalkSpeed;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AHeroCharacter::SetStateAimingInPlace()
{
	m_HeroState = EHeroState::EHS_AimingInPlace;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AHeroCharacter::SetStateAimWalking()
{
	m_HeroState = EHeroState::EHS_AimWalking;
	GetCharacterMovement()->MaxWalkSpeed = m_MaxAimWalkSpeed;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AHeroCharacter::SetStateSprinting()
{
	m_HeroState = EHeroState::EHS_Sprinting;
	GetCharacterMovement()->MaxWalkSpeed = m_MaxSprintSpeed;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AHeroCharacter::OnAxeThrown()
{
	m_HeroFlags &= ~EHeroFlags::EHF_Equipped;

	switch (m_HeroState)
	{
	case EHeroState::EHS_AimingInPlace:
		SetStateIdle();
		break;

	case EHeroState::EHS_AimWalking:
		if (SprintButtonDown())
		{
			SetStateSprinting();
		}
		else
		{
			SetStateWalking();
		}
		break;

	default:
		break;
	}
}

void AHeroCharacter::OnAxeRecallCompleted()
{
	if (m_HealthComponent->IsDead()) return;

	m_HeroFlags &= ~EHeroFlags::EHF_RecallingAxe;
	m_HeroFlags |= EHeroFlags::EHF_Equipped;
	
	if (!IsAnyMontagePlaying() && ensureAlwaysMsgf(m_AxeCatchMontage, TEXT("Axe Catch Montage is not set!")))
	{
		PlayAnimMontage(m_AxeCatchMontage);
	}

	if (AimButtonDown())
	{
		switch (m_HeroState)
		{
		case EHeroState::EHS_Idle:
			SetStateAimingInPlace();

			break;

		case EHeroState::EHS_Walking:
			SetStateAimWalking();

			break;

		default:
			break;
		}
	}
}

void AHeroCharacter::OnDeath()
{
	m_CameraBoom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	m_CameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName{"DeathCameraSocket"});
	m_CameraBoom->bEnableCameraLag = true;
	m_CameraBoom->bEnableCameraRotationLag = true;
	m_CameraBoom->CameraLagSpeed = 3.0f;
	m_CameraBoom->CameraRotationLagSpeed = 3.0f;
	m_CameraBoom->TargetArmLength *= 1.5f;
	m_CameraBoom->bUsePawnControlRotation = false;

	DisableInput(m_HeroPlayerController.Get());

	m_HeroFlags |= EHeroFlags::EHF_Stunned;
	
	CastChecked<AMain_GameMode>(GetWorld()->GetAuthGameMode())->OnPawnKilled(this);
	
	if (ensureAlwaysMsgf(m_DeathMontageInfo.m_Montage, TEXT("Montage in Death Montage Info is not set!")))
	{
		PlayAnimMontage(m_DeathMontageInfo.m_Montage, 1.0f, m_DeathMontageInfo.GetRandomSection());
	}
}

void AHeroCharacter::OnHitReactCompleted()
{
	m_HeroFlags &= ~EHeroFlags::EHF_Stunned;
}

void AHeroCharacter::OnDeathAnimCompleted()
{
	m_HeroPlayerController->RestartLevel();
}

float AHeroCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (m_HealthComponent->IsDead()) return 0.0f;

	m_HealthComponent->ReceiveDamage(Damage);

	return Damage;
}

void AHeroCharacter::ReactToHit(const FHitResult& HitResult, const AActor* const HitCauser)
{
	if (m_HealthComponent->IsDead()) return;

	m_HeroFlags |= EHeroFlags::EHF_Stunned;

	PlayAnimMontage(m_HitReactMontageInfo.m_Montage, 1.0f, m_HitReactMontageInfo.GetRandomSection());

	UGameplayStatics::SpawnEmitterAtLocation(this, m_BloodSplatterParticle, GetActorLocation(), FRotator::ZeroRotator, FVector{ m_BloodSplatterParticleScale });
}

FVector AHeroCharacter::GetPawnViewLocation() const
{
	return m_FollowCamera->GetComponentLocation();
}