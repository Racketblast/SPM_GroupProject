#include "BTTask_FireEnemyProjectile.h"

#include "AI_Controller.h"
#include "AI_Main.h"
#include "Projectile.h"
#include "PlayerCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

/* ─────────────────────────────────────────────── */

UBTTask_FireEnemyProjectile::UBTTask_FireEnemyProjectile()
{
	NodeName = TEXT("Fire Enemy Projectile (Turn & Shoot)");
}

/* ─────────────────────────────────────────────── */

EBTNodeResult::Type UBTTask_FireEnemyProjectile::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	AAI_Controller* Controller = Cast<AAI_Controller>(OwnerComp.GetAIOwner());
	if (!Controller) return EBTNodeResult::Failed;

	AAI_Main* AICharacter = Cast<AAI_Main>(Controller->GetPawn());
	if (!AICharacter || !ProjectileClass) return EBTNodeResult::Failed;

	/* ---------------------------------------------
	 *   1. Get player location and calculate direction
	 * --------------------------------------------*/
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(AICharacter, 0);
	if (!Player) return EBTNodeResult::Failed;

	const FVector AILoc     = AICharacter->GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();

	/* Face only in yaw (keep pitch/roll level) */
	FRotator LookAtYaw = (PlayerLoc - AILoc).Rotation();
	LookAtYaw.Pitch = 0.f;
	LookAtYaw.Roll  = 0.f;

	/* rotate pawn and controller */
	AICharacter->SetActorRotation(LookAtYaw);
	Controller->SetControlRotation(LookAtYaw);

	/* ---------------------------------------------
	 *   2. Spawn the projectile
	 * --------------------------------------------*/
	const FVector MuzzleLocation =
	    AILoc + AICharacter->GetActorForwardVector() * MuzzleForwardOffset +
	    FVector(0.f, 0.f, MuzzleUpOffset);

	const FVector ShotDir   = (PlayerLoc - MuzzleLocation).GetSafeNormal();
	const FRotator ShotRot  = ShotDir.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner      = AICharacter;
	SpawnParams.Instigator = AICharacter;

	AProjectile* Proj = AICharacter->GetWorld()->SpawnActor<AProjectile>(
	    ProjectileClass, MuzzleLocation, ShotRot, SpawnParams);

	if (Proj)
	{
		/* Pass damage from AI */
		Proj->ProjectileDamage = AICharacter->AIDamage;

		/* Kick off movement */
		if (UProjectileMovementComponent* Move = Proj->ProjectileComponent)
		{
			Move->Velocity = ShotDir * Move->InitialSpeed;
		}
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
