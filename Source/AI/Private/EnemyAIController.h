// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:

	AEnemyAIController();

private:

	UPROPERTY(EditAnywhere, Category = "AI", meta = (DisplayName = "Behavior Tree"))
	TObjectPtr<UBehaviorTree> m_BehaviorTree{ nullptr };

protected:

	virtual void OnPossess(APawn* InPawn) override;
};
