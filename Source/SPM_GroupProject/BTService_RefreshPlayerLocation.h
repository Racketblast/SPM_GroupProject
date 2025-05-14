// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_RefreshPlayerLocation.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API UBTService_RefreshPlayerLocation : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_RefreshPlayerLocation();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector UpdatePlayerLocationKey;
};
