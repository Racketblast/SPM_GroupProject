#include "BTTask_FireEnemyProjectile.h"
#include "AI_Controller.h"
#include "AI_Main.h"
#include "Projectile.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

UBTTask_FireEnemyProjectile::UBTTask_FireEnemyProjectile()
{
	NodeName = TEXT("Fire Enemy Projectile");
}

EBTNodeResult::Type UBTTask_FireEnemyProjectile::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAI_Controller* Controller = Cast<AAI_Controller>(OwnerComp.GetAIOwner()))
	{
		if (AAI_Main* AICharacter = Cast<AAI_Main>(Controller->GetPawn()))
		{
			ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
			if (!Player || !ProjectileClass) return EBTNodeResult::Failed;

			FVector MuzzleLocation = AICharacter->GetActorLocation() + AICharacter->GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
			FVector Direction = (Player->GetActorLocation() - MuzzleLocation).GetSafeNormal();
			FRotator FireRotation = Direction.Rotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = AICharacter;
			SpawnParams.Instigator = AICharacter;

			AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, MuzzleLocation, FireRotation, SpawnParams);
			if (Projectile && Projectile->ProjectileComponent)
			{
				Projectile->ProjectileComponent->Velocity = Direction * Projectile->ProjectileComponent->InitialSpeed;
			}

			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
