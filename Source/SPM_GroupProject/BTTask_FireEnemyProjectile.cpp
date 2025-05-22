#include "BTTask_FireEnemyProjectile.h"

#include "AI_Controller.h"
#include "AI_Main.h"
#include "Projectile.h"
#include "PlayerCharacter.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"   

/* ─────────────────────────────────────────────── */
UBTTask_FireEnemyProjectile::UBTTask_FireEnemyProjectile()
{
	NodeName = TEXT("Fire Enemy Projectile (Turn & Shoot)");
}

/* ─────────────────────────────────────────────── */
EBTNodeResult::Type UBTTask_FireEnemyProjectile::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* )
{
	AAI_Controller* Controller = Cast<AAI_Controller>(OwnerComp.GetAIOwner());
	if (!Controller) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAI_Main*             AI = Cast<AAI_Main>(Controller->GetPawn());
	if (!AI || !ProjectileClass || !BB) return EBTNodeResult::Failed;

	/*  “IsFiring” true for given duration second */
	BB->SetValueAsBool(FName("IsFiring"), true);

	FTimerHandle TmpHandle;
	AI->GetWorldTimerManager().SetTimer(
	    TmpHandle,
	    FTimerDelegate::CreateLambda([BB]()
	    {
		    if (BB) BB->SetValueAsBool(FName("IsFiring"), false);
	    }),
	    5.0f, false);      // duration

	/*  Rotate toward player */
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(AI, 0);
	if (!Player) return EBTNodeResult::Failed;

	const FVector AILoc     = AI->GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();

	FRotator YawOnly = (PlayerLoc - AILoc).Rotation();
	YawOnly.Pitch = 0.f;
	YawOnly.Roll  = 0.f;
	AI->SetActorRotation(YawOnly);
	Controller->SetControlRotation(YawOnly);
	
	const FVector MuzzleLoc =
	    AILoc + AI->GetActorForwardVector() * MuzzleForwardOffset +
	    FVector(0.f, 0.f, MuzzleUpOffset);

	const FVector Dir = (PlayerLoc - MuzzleLoc).GetSafeNormal();
	const FRotator ShotRot = Dir.Rotation();

	FActorSpawnParameters Params;
	Params.Owner      = AI;
	Params.Instigator = AI;

	AProjectile* Proj = AI->GetWorld()->SpawnActor<AProjectile>(
	    ProjectileClass, MuzzleLoc, ShotRot, Params);

	if (Proj)
	{
		Proj->ProjectileDamage = AI->AIDamage;
		if (UProjectileMovementComponent* Move = Proj->ProjectileComponent)
		{
			Move->Velocity = Dir * Move->InitialSpeed;
		}
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
