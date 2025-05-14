// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CooldownTimer.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API UBTDecorator_CooldownTimer : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CooldownTimer();

protected:
	UPROPERTY(EditAnywhere, Category = "Cooldown")
	FName TimeKey;

	UPROPERTY(EditAnywhere, Category = "Cooldown")
	float Cooldown = 2.0f;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
};
