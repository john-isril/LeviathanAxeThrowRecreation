// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HittableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHittableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INTERFACES_API IHittableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void ReactToHit(const FHitResult& HitResult, const AActor* const HitCauser) = 0;
};
