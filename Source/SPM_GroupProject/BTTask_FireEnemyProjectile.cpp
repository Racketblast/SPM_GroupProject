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
EBTNodeResult::Type UBTTask_FireEnemyProjectile::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* )
{
	//Get the AI controller
	AAI_Controller* Controller = Cast<AAI_Controller>(OwnerComp.GetAIOwner());
	if (!Controller) return EBTNodeResult::Failed;

	//Get the blackboard
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	//Get the AI character
	AAI_Main* AI = Cast<AAI_Main>(Controller->GetPawn());
	//Validate
	if (!AI || !ProjectileClass || !BB) return EBTNodeResult::Failed;

	/*  “IsFiring” true*/
	BB->SetValueAsBool(FName("IsFiring"), true);
	
	if (!AI->GetWorldTimerManager().IsTimerActive(ResetFireHandle))
	{
		// Capture the BB weakly so the delegate won't extend its lifetime.
		TWeakObjectPtr<UBlackboardComponent> WeakBB(BB);

		//Skapa timer delegate som senare ska kallas från timern
		FTimerDelegate ResetDel = FTimerDelegate::CreateWeakLambda(AI /*owner tracked "weakly" by delegate, Motvärkar memory leaks*/, [WeakBB]()
			{
				if (WeakBB.IsValid())//Dubbelkollar att den finns kvar
				{
					WeakBB->SetValueAsBool(TEXT("IsFiring"), false);
				}
			});
		//Starta timer
		AI->GetWorldTimerManager().SetTimer(ResetFireHandle,ResetDel,FiringCooldown,false);
	}

	/*  Rotate toward player */
	// Hämta player character
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(AI, 0);
	if (!Player) return EBTNodeResult::Failed;

	//Hämta world locations(AI och spelaren)
	const FVector AILoc     = AI->GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();

	//Räkna ut rotationen mot spelaren
	FRotator YawRot = (PlayerLoc - AILoc).Rotation();
	//Undvik pitch och roll
	YawRot.Pitch = 0.f;
	YawRot.Roll  = 0.f;
	//Kör rotation
	AI->SetActorRotation(YawRot);
	Controller->SetControlRotation(YawRot);

	
	
	//Räkna ut Muzzle location
	const FVector MuzzleLoc =
			AILoc + AI->GetActorForwardVector() * MuzzleForwardOffset +
			FVector(0.f, 0.f, MuzzleUpOffset);

	//Räkna ut riktning och rotation
	const FVector Dir = (PlayerLoc - MuzzleLoc).GetSafeNormal();//Normaliserad vector riktad mot spelaren
	const FRotator ShotRot = Dir.Rotation();//Converterar dir till en rotator för siktning

	FActorSpawnParameters Params;
	Params.Owner      = AI;//Vem äger projectile(för damage logik etc)
	Params.Instigator = AI;//Vilken pawn som spawnar den(AI)

	//Spawna projektilen
	AProjectile* Proj = AI->GetWorld()->SpawnActor<AProjectile>(
	    ProjectileClass, MuzzleLoc, ShotRot, Params);

	//velocity
	if (Proj)
	{
		if (UProjectileMovementComponent* Move = Proj->ProjectileComponent) //valid "UProjectileMovementComponent"?, "Movement component" som finns i "Projectile component" i blueprinten
		{
			Move->Velocity = Dir * Move->InitialSpeed; //Initial speed finns i "Projectile component" i blueprinten
		}
		return EBTNodeResult::Succeeded;
	}
	//Om proj inte kan spawnas
	return EBTNodeResult::Failed;
}