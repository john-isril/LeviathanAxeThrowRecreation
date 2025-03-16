// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AxeThrowAnimNotify.h"
#include "LightAxeThrowAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class AXECOMBAT_API ULightAxeThrowAnimNotify : public UAxeThrowAnimNotify
{
	GENERATED_BODY()

public:
	ULightAxeThrowAnimNotify();
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
