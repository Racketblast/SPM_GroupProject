#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FireEnemyProjectile.generated.h"

class AProjectile;

/**
 * Fires a projectile at the player and first rotates the AI toward the target.
 */
UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FireEnemyProjectile : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FireEnemyProjectile();

	/** Behaviour-tree entry point */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8*                 NodeMemory) override;

	/** Projectile blueprint to spawn */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass = nullptr;

	/** How far in front of the AI the muzzle is (cm) */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float MuzzleForwardOffset = 100.f;

	/** Vertical offset for the muzzle (cm) */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float MuzzleUpOffset = 50.f;
};
