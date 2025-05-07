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

protected:

	UPROPERTY(EditAnywhere, Category = "Flight")
	float ZOffset;

	UPROPERTY(EditAnywhere, Category = "Randomization")
	bool bAddRandomOffset;

	UPROPERTY(EditAnywhere, Category = "Randomization", meta = (EditCondition = "bAddRandomOffset"))
	float RandomRadius;
};
