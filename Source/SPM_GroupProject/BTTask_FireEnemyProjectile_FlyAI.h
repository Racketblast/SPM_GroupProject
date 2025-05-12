// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FireEnemyProjectile_FlyAI.generated.h"

class AProjectile;

UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FireEnemyProjectile_FlyAI : public UBTTaskNode
{
	GENERATED_BODY()

	
public:
	UBTTask_FireEnemyProjectile_FlyAI();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;
	
};
