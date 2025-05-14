// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FlyToTarget.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FlyToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FlyToTarget();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {}

private:
	TArray<FVector> CurrentPath;
	int32 CurrentPathIndex = 0;
	float AcceptanceRadius = 100.f;

	FVector GetNextPathPoint() const;
	void MoveToward(UBehaviorTreeComponent& OwnerComp, float DeltaSeconds);
};
