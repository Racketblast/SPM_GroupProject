#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FireEnemyProjectile.generated.h"

class AProjectile;

UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FireEnemyProjectile : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FireEnemyProjectile();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;
};
