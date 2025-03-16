// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AxeThrowAnimNotify.h"
#include "HeavyAxeThrowAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class AXECOMBAT_API UHeavyAxeThrowAnimNotify : public UAxeThrowAnimNotify
{
	GENERATED_BODY()

public:
	UHeavyAxeThrowAnimNotify();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
