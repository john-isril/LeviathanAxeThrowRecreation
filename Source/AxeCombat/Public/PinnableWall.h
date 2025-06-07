// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinnableWall.generated.h"

/*
* Class for objects that enemies can be pinned against with the Leviathan Axe.
*/
UCLASS()
class AXECOMBAT_API APinnableWall : public AActor
{
	GENERATED_BODY()
	
public:	

	APinnableWall();

private:

	UPROPERTY(VisibleAnywhere, Category = "Properties", meta = (DisplayName = "Mesh"))
	TObjectPtr<UStaticMeshComponent> m_Mesh{ nullptr };

};
