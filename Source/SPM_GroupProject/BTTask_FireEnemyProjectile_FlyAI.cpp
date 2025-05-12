// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FireEnemyProjectile_FlyAI.h"
#include "FlyingAI_Controller.h"
#include "FlyingEnemyAI.h"
#include "Projectile.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

UBTTask_FireEnemyProjectile_FlyAI::UBTTask_FireEnemyProjectile_FlyAI()
{
	NodeName = TEXT("Fire Enemy Projectile (Flying)");
}

EBTNodeResult::Type UBTTask_FireEnemyProjectile_FlyAI::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AFlyingAI_Controller* Controller = Cast<AFlyingAI_Controller>(OwnerComp.GetAIOwner()))
	{
		if (AFlyingEnemyAI* AICharacter = Cast<AFlyingEnemyAI>(Controller->GetPawn()))
		{
			if (!AICharacter->IsFireCooldownElapsed())
			{
				return EBTNodeResult::Failed;
			}

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

			AICharacter->NotifyFired(); 

			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}

