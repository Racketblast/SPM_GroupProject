// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_RepositionFlyingEnemy.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API UBTTask_RepositionFlyingEnemy : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_RepositionFlyingEnemy();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Reposition")
	float RepositionRadius = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Reposition")
	float ZOffsetMin = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Reposition")
	float ZOffsetMax = 400.0f;
	
};
