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

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector MoveToLocationKey;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 100.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float StuckCheckInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float StuckMovementThreshold = 10.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float BackoffDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float BackoffCooldownTime = 1.5f;

	void MoveTowardTarget(APawn* Pawn, const FVector& TargetLocation);

	void CheckIfStuck(UBehaviorTreeComponent& OwnerComp, APawn* Pawn, const FVector& TargetLocation);

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float TargetUpdateInterval = 2.0f;

	float TimeSinceLastTargetUpdate = 0.0f;

private:
	FVector LastLocation;
	float TimeSinceLastMove = 0.0f;
	bool bBackingOff = false;
	float BackoffElapsed = 0.0f;
};
