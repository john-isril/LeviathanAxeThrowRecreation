// Fill out your copyright notice in the Description page of Project Settings.


#include "HeavyAxeThrowAnimNotify.h"

UHeavyAxeThrowAnimNotify::UHeavyAxeThrowAnimNotify()
{
}

void UHeavyAxeThrowAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	OnNotified.ExecuteIfBound(true);
}