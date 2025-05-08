// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindPlayerLocation_Flying.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FindPlayerLocation_Flying : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_FindPlayerLocation_Flying(FObjectInitializer const& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	static bool IsLocationClear(UWorld* World, const FVector& Location, AActor* IgnoredActor, float Distance, float Clearance);

protected:

	UPROPERTY(EditAnywhere, Category = "Flight")
	float ZOffset;

	UPROPERTY(EditAnywhere, Category = "Randomization")
	bool bAddRandomOffset;

	UPROPERTY(EditAnywhere, Category = "Randomization", meta = (EditCondition = "bAddRandomOffset"))
	float RandomRadius;

	UPROPERTY(EditAnywhere, Category = "Obstacle Avoidance")
	float ObstacleCheckDistance = 200.f; // how far to check for nearby obstacles

	UPROPERTY(EditAnywhere, Category = "Obstacle Avoidance")
	float ObstacleClearance = 100.f;     // how close is too close to accept
};
