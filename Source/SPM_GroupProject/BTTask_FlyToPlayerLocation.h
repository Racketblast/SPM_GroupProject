// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FlyToPlayerLocation.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FlyToPlayerLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FlyToPlayerLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {}

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveToLocationKey;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 100.f;
	
};
