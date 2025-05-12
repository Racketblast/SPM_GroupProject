// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingEnemyAI.h"
#include "FlyingAI_Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

AFlyingEnemyAI::AFlyingEnemyAI()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AFlyingAI_Controller::StaticClass();

}

void AFlyingEnemyAI::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
}

void AFlyingEnemyAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

/*#if WITH_EDITOR
	DrawFlyableZRange();
#endif*/

	if (!CurrentTargetLocation.IsNearlyZero())
	{
		FVector CurrentLocation = GetActorLocation();
		
		if (CurrentTargetLocation.Z < CurrentLocation.Z - 10.0f) 
		{
			StuckCheckTimer += DeltaTime;

			if (StuckCheckTimer >= 1.0f) 
			{
				float ZDelta = FMath::Abs(CurrentLocation.Z - LastLocation.Z);

				if (ZDelta < 5.0f)
				{
					bIsDescendingStuck = true;
					TryLateralUnstick(); 
				}
				else
				{
					bIsDescendingStuck = false;
				}

				LastLocation = CurrentLocation;
				StuckCheckTimer = 0.0f;
			}
		}
	}
}

void AFlyingEnemyAI::SetMaxAltitude(float Altitude)
{
	MaxAltitude = Altitude;
}

float AFlyingEnemyAI::GetMaxAltitude() const
{
	return MaxAltitude;
}

void AFlyingEnemyAI::SetMinAltitude(float Altitude)
{
	MinAltitude = Altitude;
}

float AFlyingEnemyAI::GetMinAltitude() const
{
	return MinAltitude;
}

void AFlyingEnemyAI::DrawFlyableZRange()
{
	FVector ActorLocation = GetActorLocation();

	// Debug sphere för min altitude 
	FVector MinZLocation = FVector(ActorLocation.X, ActorLocation.Y, MinAltitude);
	DrawDebugSphere(GetWorld(), MinZLocation, 40.f, 12, FColor::Red, false, -1.f, 0, 2.f);

	// Debug sphere för max altitude
	FVector MaxZLocation = FVector(ActorLocation.X, ActorLocation.Y, MaxAltitude);
	DrawDebugSphere(GetWorld(), MaxZLocation, 40.f, 12, FColor::Green, false, -1.f, 0, 2.f);

	// Debug linje som är mellan dem två debug sphere 
	DrawDebugLine(GetWorld(), MinZLocation, MaxZLocation, FColor::Yellow, false, -1.f, 0, 1.f);
}

void AFlyingEnemyAI::TryLateralUnstick()
{
	FVector LateralDirection;

	// Väljer antingen vänster eller höger
	if (FMath::RandBool())
	{
		LateralDirection = GetActorRightVector();
	}
	else
	{
		LateralDirection = -GetActorRightVector();
	}

	FVector NewTarget = GetActorLocation() + LateralDirection * 300.0f; // rör sig till sidan

	// liten push uppåt
	NewTarget.Z += 100.0f;

	FVector Direction = (NewTarget - GetActorLocation()).GetSafeNormal();
	AddMovementInput(Direction, 1.0f, false);
}

void AFlyingEnemyAI::SetCurrentTargetLocation(const FVector& NewTarget)
{
	CurrentTargetLocation = NewTarget;
}

FVector AFlyingEnemyAI::GetCurrentTargetLocation() const
{
	return CurrentTargetLocation;
}

void AFlyingEnemyAI::NotifyTeleported()
{
	LastTeleportTime = GetWorld()->GetTimeSeconds();
}

bool AFlyingEnemyAI::CanShoot() const
{
	return (GetWorld()->GetTimeSeconds() - LastTeleportTime) >= PostTeleportFireDelay;
}