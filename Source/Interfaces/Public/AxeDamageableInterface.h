// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AxeDamageableInterface.generated.h"

/*The different types of actions that can be performed when hit by a Leviathan Axe.*/
enum class EAxeHitType : uint8
{
	EAHT_None,
	EAHT_KnockBack,
	EAHT_AttachToAndFreeze,
	EAHT_DetachFrom,
	EAHT_Carry,
	EAHT_PinToWall,
	EAHT_Drop,
	EAHT_Max
};

DECLARE_DELEGATE(FOnFreezeCompletedSignature);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAxeDamageableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INTERFACES_API IAxeDamageableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/*Reaction to being hit by a Leviathan Axe.*/
	virtual bool OnHit(const FHitResult& HitResult, AActor* Causer, EAxeHitType AxeHitType) = 0;
	
	virtual USkeletalMeshComponent* GetSkeletalMeshComponent() const = 0;
	
	virtual bool IsAlive() const = 0;

public:
	
	FOnFreezeCompletedSignature OnFreezeCompleted;

private:
	
	virtual void Freeze() = 0;
	
	virtual void Unfreeze() = 0;
};
