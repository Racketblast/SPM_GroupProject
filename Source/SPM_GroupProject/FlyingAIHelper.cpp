// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingAIHelper.h"
#include "FlyingEnemyAI.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

FVector FlyingAIHelper::ComputeSmartTargetLocation(
	UWorld* World,
	ACharacter* Player,
	AFlyingEnemyAI* FlyingEnemy,
	const FVector& FromLocation,
	float ZOffset,
	bool bAddRandomOffset,
	float RandomRadius,
	float ObstacleCheckDistance,
	float ObstacleClearance)
{
	FVector PlayerLocation = Player->GetActorLocation();
	FVector TargetLocation = PlayerLocation;

	// Line trace för att kolla taket
	FHitResult CeilingHit;
	FCollisionQueryParams CeilingParams;
	CeilingParams.AddIgnoredActor(Player);
	CeilingParams.AddIgnoredActor(FlyingEnemy);

	FVector CeilingTraceEnd = PlayerLocation + FVector(0.f, 0.f, ZOffset + 100.f);
	bool bCeilingClose = false;

	if (World->LineTraceSingleByChannel(CeilingHit, PlayerLocation, CeilingTraceEnd, ECC_Visibility, CeilingParams))
	{
		if (CeilingHit.Distance < ZOffset + 50.f)
		{
			bCeilingClose = true;
		}
	}

	if (!bCeilingClose)
	{
		TargetLocation.Z += ZOffset;
	}
	else
	{
		TargetLocation.Z += FMath::Clamp(CeilingHit.Distance - 100.f, 0.f, ZOffset);
	}

	// Sätter max och min höjd som fienden får flyga till 
	if (FlyingEnemy)
	{
		TargetLocation.Z = FMath::Clamp(TargetLocation.Z, FlyingEnemy->GetMinAltitude(), FlyingEnemy->GetMaxAltitude());
	}

	// Lägger till en random offset
	if (bAddRandomOffset)
	{
		TargetLocation += FVector(
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius * 0.5f, RandomRadius * 0.5f)
		);
	}

	// Kållar efter både tak och golv för att undvika att åka in i de
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(FlyingEnemy);

	FVector Down = TargetLocation - FVector(0.f, 0.f, 10000.f);
	FVector Up = TargetLocation + FVector(0.f, 0.f, 10000.f);

	if (World->LineTraceSingleByChannel(HitResult, TargetLocation, Down, ECC_Visibility, Params))
	{
		TargetLocation.Z = FMath::Max(TargetLocation.Z, HitResult.Location.Z + 200.f);
	}
	if (World->LineTraceSingleByChannel(HitResult, TargetLocation, Up, ECC_Visibility, Params))
	{
		TargetLocation.Z = FMath::Min(TargetLocation.Z, HitResult.Location.Z - 200.f);
	}

	// Kållar efter hinder
	bool bValidLocation = IsLocationClear(World, TargetLocation, FlyingEnemy, ObstacleCheckDistance, ObstacleClearance);

	if (!bValidLocation && bAddRandomOffset)
	{
		FVector InitialRetryLocation = PlayerLocation + FVector(
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius, RandomRadius),
			ZOffset
		);

		if (FlyingEnemy)
		{
			InitialRetryLocation.Z = FMath::Clamp(InitialRetryLocation.Z, FlyingEnemy->GetMinAltitude(), FlyingEnemy->GetMaxAltitude());
		}

		if (IsLocationClear(World, InitialRetryLocation, FlyingEnemy, ObstacleCheckDistance, ObstacleClearance))
		{
			TargetLocation = InitialRetryLocation;
		}
		/*else
		{
			const float RetryDistance = 50.f;
			bool bFoundValidPath = false;

			// List of deliberate directions to try in order
			FVector Forward = FlyingEnemy->GetActorForwardVector();
			FVector Right = FlyingEnemy->GetActorRightVector();

			TArray<FVector> ProbeDirections = {
				Forward,
				Right,
				-Right,
				(Forward + Right).GetSafeNormal(),
				(Forward - Right).GetSafeNormal(),
				-Forward
			};


			for (const FVector& Dir : ProbeDirections)
			{
				FVector RetryLocation = FromLocation + Dir * RetryDistance;
				RetryLocation.Z = TargetLocation.Z;

				if (FlyingEnemy)
				{
					RetryLocation.Z = FMath::Clamp(RetryLocation.Z, FlyingEnemy->GetMinAltitude(), FlyingEnemy->GetMaxAltitude());
				}

				if (IsLocationClear(World, RetryLocation, FlyingEnemy, ObstacleCheckDistance, ObstacleClearance) &&
					IsPathClear(World, FromLocation, RetryLocation, FlyingEnemy))
				{
					TargetLocation = RetryLocation;
					bFoundValidPath = true;
					break;
				}
			}

			// Final fallback if all retries failed
			if (!bFoundValidPath)
			{
				FVector Direction = (PlayerLocation - FromLocation).GetSafeNormal();
				TargetLocation = FromLocation + Direction * 500.f;
				TargetLocation.Z += ZOffset;

				if (FlyingEnemy)
				{
					TargetLocation.Z = FMath::Clamp(TargetLocation.Z, FlyingEnemy->GetMinAltitude(), FlyingEnemy->GetMaxAltitude());
				}
			}
		}*/
		else
		{
			// Fallback 
			FVector Direction = (PlayerLocation - FromLocation).GetSafeNormal();
			TargetLocation = FromLocation + Direction * 500.f;
			TargetLocation.Z += ZOffset;

			if (FlyingEnemy)
			{
				TargetLocation.Z = FMath::Clamp(TargetLocation.Z, FlyingEnemy->GetMinAltitude(), FlyingEnemy->GetMaxAltitude());
			}
		}
	}
	
	// DrawDebugSphere(World, TargetLocation, 30.f, 12, FColor::Green, false, 2.f);

	
	if (FlyingEnemy)
	{
		FlyingEnemy->IsMoving();
	}

	return TargetLocation;
}

bool FlyingAIHelper::IsLocationClear(
	UWorld* World,
	const FVector& Location,
	AActor* IgnoredActor,
	float Distance,
	float Clearance)
{
	TArray<FVector> Directions = {
		FVector::ForwardVector, -FVector::ForwardVector,
		FVector::RightVector,   -FVector::RightVector,
		FVector::UpVector,      -FVector::UpVector
	};

	for (const FVector& Dir : Directions)
	{
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(IgnoredActor);
		if (World->LineTraceSingleByChannel(Hit, Location, Location + Dir * Distance, ECC_Visibility, Params))
		{
			if (Hit.Distance < Clearance)
			{
				return false;
			}
		}
	}
	return true;
}

bool FlyingAIHelper::IsPathClear(
	UWorld* World,
	const FVector& FromLocation,
	const FVector& ToLocation,
	AFlyingEnemyAI* FlyingEnemy)
{
	if (!World || !FlyingEnemy) return false;

	UCapsuleComponent* Capsule = FlyingEnemy->GetCapsuleComponent();
	if (!Capsule) return false;

	float Radius = Capsule->GetScaledCapsuleRadius();
	float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(FlyingEnemy);
	Params.bTraceComplex = false;

	TArray<AActor*> AllFlyingEnemies;
	UGameplayStatics::GetAllActorsOfClass(World, AFlyingEnemyAI::StaticClass(), AllFlyingEnemies);

	for (AActor* Enemy : AllFlyingEnemies)
	{
		Params.AddIgnoredActor(Enemy);
	}

	// Define the capsule shape
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);

	FHitResult Hit;
	bool bHit = World->SweepSingleByChannel(
		Hit,
		FromLocation,
		ToLocation,
		FQuat::Identity,
		ECC_Visibility,
		CollisionShape,
		Params
	);


	// debug 
	
	DrawDebugCapsule(World, FromLocation, HalfHeight, Radius, FQuat::Identity, FColor::Blue, false, 1.f);
	DrawDebugCapsule(World, ToLocation, HalfHeight, Radius, FQuat::Identity, FColor::Red, false, 1.f);
	DrawDebugLine(World, FromLocation, ToLocation, bHit ? FColor::Red : FColor::Green, false, 1.f, 0, 2.f);
	


	return !bHit;
}
