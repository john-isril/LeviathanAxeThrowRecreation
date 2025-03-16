// Fill out your copyright notice in the Description page of Project Settings.


#include "LightAxeThrowAnimNotify.h"

ULightAxeThrowAnimNotify::ULightAxeThrowAnimNotify()
{
}

void ULightAxeThrowAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	OnNotified.ExecuteIfBound(false);
}
