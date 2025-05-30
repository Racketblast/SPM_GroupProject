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

	/** Seconds before IsFiring is automatically reset to false. */
	UPROPERTY(EditAnywhere, Category="Firing")
	float FiringCooldown = 5.f;

	/** Forward offset from pawn origin to muzzle (cm). */
	UPROPERTY(EditAnywhere, Category="Firing")
	float MuzzleForwardOffset = 100.f;

	/** Vertical offset from pawn origin to muzzle (cm). */
	UPROPERTY(EditAnywhere, Category="Firing")
	float MuzzleUpOffset = 50.f;

	/** Projectile Blueprint/Class to spawn. Must derive from AProjectile. */
	UPROPERTY(EditAnywhere, Category="Firing")
	TSubclassOf<class AProjectile> ProjectileClass;

protected:
	/** Handle for the cool‑down timer so only one is ever active. */
	UPROPERTY()
	FTimerHandle ResetFireHandle;

private:
	/** Main execution entry point called by the behavior tree. */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
											uint8* NodeMemory) override;
};

