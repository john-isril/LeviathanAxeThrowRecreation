// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AxeThrowAnimNotify.generated.h"

DECLARE_DELEGATE_OneParam(FOnNotifiedSignature, bool bIsHeavy)

/**
 * 
 */
UCLASS()
class AXECOMBAT_API UAxeThrowAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAxeThrowAnimNotify();

public:
	FOnNotifiedSignature OnNotified;
	
};
