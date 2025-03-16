// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySword.h"
#include <Components/BoxComponent.h>
#include <Kismet/GameplayStatics.h>
#include "HittableInterface.h"


AEnemySword::AEnemySword()
{
	PrimaryActorTick.bCanEverTick = true;

	m_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(m_Mesh);

	m_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	m_DamageCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	m_DamageCollisionBox->SetupAttachment(GetRootComponent());
}

void AEnemySword::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	EnableDamage(false);

	m_DamageCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &AEnemySword::OnDamageCollisionBoxOverlap);
}

void AEnemySword::EnableDamage(bool Enable)
{
	if (Enable)
	{
		m_DamageCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		if (ensureAlwaysMsgf(m_SwingSound, TEXT("Swing Sound is not set!")) && ensureAlwaysMsgf(m_SoundAttenuation, TEXT("Sound Attenuation is not set!")))
		{
			UGameplayStatics::PlaySoundAtLocation(this, m_SwingSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, m_SoundAttenuation);
		}
	}
	else
	{
		m_DamageCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AEnemySword::DropFromParent()
{
	EnableDamage(false);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	m_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	m_Mesh->SetSimulatePhysics(true);

	static constexpr float DropImpulseMag{ 15.0f };
	m_Mesh->AddImpulse(GetActorForwardVector() * DropImpulseMag);
}

void AEnemySword::OnDamageCollisionBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor->GetClass() != GetOwner()->GetClass()))
	{
		m_DamageCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		UGameplayStatics::ApplyDamage(OtherActor, m_DamageAmount, nullptr, GetOwner(), UDamageType::StaticClass());

		if (IHittableInterface* const HittableInterfaceActor{ Cast<IHittableInterface>(OtherActor) })
		{
			HittableInterfaceActor->ReactToHit(SweepResult, this);
		}
	}
}

