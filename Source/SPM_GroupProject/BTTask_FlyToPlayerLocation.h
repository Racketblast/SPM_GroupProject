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

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 100.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float StuckCheckInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float StuckMovementThreshold = 10.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float BackoffDistance = 200.f;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector MoveToLocationKey;

private:
	FVector LastLocation;
	float TimeSinceLastMove = 0.f;
	bool bBackingOff = false;
	
};
