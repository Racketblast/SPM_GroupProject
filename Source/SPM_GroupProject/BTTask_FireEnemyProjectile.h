// ───────────────────────────────────────────────────────────────────────────── //
//  Header – placed at bottom so this file is self‑contained for the example.
//  In production keep .h and .cpp separate.  The header is included above, so
//  *this* copy is purely illustrative.
// ───────────────────────────────────────────────────────────────────────────── //
   // <–––– REMOVE THIS BLOCK IN REAL HEADER FILE >
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FireEnemyProjectile.generated.h"

/**
 * Face player, fire a projectile, and manage the IsFiring blackboard flag.
 */
UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FireEnemyProjectile : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FireEnemyProjectile();
	
	UPROPERTY(EditAnywhere, Category="Firing")
	float FiringCooldown = 5.f;
	
	UPROPERTY(EditAnywhere, Category="Firing")
	float MuzzleForwardOffset = 100.f;

	UPROPERTY(EditAnywhere, Category="Firing")
	float MuzzleUpOffset = 50.f;
	
	UPROPERTY(EditAnywhere, Category="Firing")
	TSubclassOf<class AProjectile> ProjectileClass;

protected:
	UPROPERTY()
	FTimerHandle ResetFireHandle;

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;
};

