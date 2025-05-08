// Fill out your copyright notice in the Description page of Project Settings.
#include "BTTask_FindPlayerLocation_Flying.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "FlyingEnemyAI.h"

UBTTask_FindPlayerLocation_Flying::UBTTask_FindPlayerLocation_Flying(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Find Player Location (Flying)";

	ZOffset = 300.0f;
	bAddRandomOffset = true;
	RandomRadius = 200.0f;
	
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindPlayerLocation_Flying, BlackboardKey));
}

EBTNodeResult::Type UBTTask_FindPlayerLocation_Flying::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!OwnerComp.GetBlackboardComponent())
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return EBTNodeResult::Failed;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector TargetLocation = PlayerLocation;

	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	AFlyingEnemyAI* FlyingEnemy = Cast<AFlyingEnemyAI>(AIPawn);

	// Line trace uppåt för att titta efter tak åvanför spelaren
	FHitResult CeilingHit;
	FCollisionQueryParams CeilingParams;
	CeilingParams.AddIgnoredActor(Player);
	CeilingParams.AddIgnoredActor(AIPawn);

	FVector CeilingTraceStart = PlayerLocation;
	FVector CeilingTraceEnd = CeilingTraceStart + FVector(0.f, 0.f, ZOffset + 100.f); // kållar lite över z-offset

	bool bCeilingClose = false;

	if (GetWorld()->LineTraceSingleByChannel(CeilingHit, CeilingTraceStart, CeilingTraceEnd, ECC_Visibility, CeilingParams))
	{
		float CeilingDistance = CeilingHit.Distance;
		if (CeilingDistance < ZOffset + 50.f) // inte tillräckligt med plats över spelaren
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
		// Förminskar offset om det inte finns tillräckligt med plats
		TargetLocation.Z += FMath::Clamp(CeilingHit.Distance - 100.f, 0.f, ZOffset);
	}

	if (FlyingEnemy)
	{
		TargetLocation.Z = FMath::Min(TargetLocation.Z, FlyingEnemy->GetMaxAltitude());
	}
	if (FlyingEnemy)
	{
		TargetLocation.Z = FMath::Max(TargetLocation.Z, FlyingEnemy->GetMinAltitude());
	}

	if (bAddRandomOffset)
	{
		FVector RandomOffset(
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius * 0.5f, RandomRadius * 0.5f)
		);
		TargetLocation += RandomOffset;
	}

	// Tittar efter tak och golv 
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AIPawn);

	FVector Start = TargetLocation;
	FVector Down = Start - FVector(0.f, 0.f, 10000.f);
	FVector Up = Start + FVector(0.f, 0.f, 10000.f);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, Down, ECC_Visibility, Params))
	{
		TargetLocation.Z = FMath::Max(TargetLocation.Z, HitResult.Location.Z + 200.f);
	}

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, Up, ECC_Visibility, Params))
	{
		TargetLocation.Z = FMath::Min(TargetLocation.Z, HitResult.Location.Z - 200.f);
	}

	// Kållar efter hinder
	bool bValidLocation = IsLocationClear(GetWorld(), TargetLocation, AIPawn, ObstacleCheckDistance, ObstacleClearance);

	if (!bValidLocation && bAddRandomOffset)
	{
		// Försöker igen med en annan random offset
		FVector RetryOffset(
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius * 0.5f, RandomRadius * 0.5f)
		);
		FVector RetryLocation = Player->GetActorLocation() + RetryOffset;
		RetryLocation.Z += ZOffset;

		if (FlyingEnemy)
		{
			RetryLocation.Z = FMath::Min(RetryLocation.Z, FlyingEnemy->GetMaxAltitude());
		}
		if (FlyingEnemy)
		{
			TargetLocation.Z = FMath::Max(TargetLocation.Z, FlyingEnemy->GetMinAltitude());
		}

		if (IsLocationClear(GetWorld(), RetryLocation, AIPawn, ObstacleCheckDistance, ObstacleClearance))
		{
			TargetLocation = RetryLocation;
			bValidLocation = true;
		}
	}

	// fallback, ifall fienden inte hittar en valid location
	if (!bValidLocation)
	{
		// Rör sig mot spelaren, 500 units ifrån sig själv
		FVector Direction = (Player->GetActorLocation() - AIPawn->GetActorLocation()).GetSafeNormal();
		TargetLocation = AIPawn->GetActorLocation() + Direction * 500.f;
		TargetLocation.Z += ZOffset; // så att fienden stannar i luften

		if (FlyingEnemy)
		{
			TargetLocation.Z = FMath::Min(TargetLocation.Z, FlyingEnemy->GetMaxAltitude());
		}
		if (FlyingEnemy)
		{
			TargetLocation.Z = FMath::Max(TargetLocation.Z, FlyingEnemy->GetMinAltitude());
		}

		UE_LOG(LogTemp, Warning, TEXT("Fallback target used at location: %s"), *TargetLocation.ToString());
	}


	// Debug, ska antagligen bortkommentera detta innan speltest
	DrawDebugSphere(GetWorld(), TargetLocation, 30.f, 12, FColor::Green, false, 2.0f);

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}


bool UBTTask_FindPlayerLocation_Flying::IsLocationClear(UWorld* World, const FVector& Location, AActor* IgnoredActor, float Distance, float Clearance)
{
	TArray<FVector> Directions = {
		FVector::ForwardVector,
		-FVector::ForwardVector,
		FVector::RightVector,
		-FVector::RightVector,
		FVector::UpVector,
		-FVector::UpVector
	};

	for (const FVector& Dir : Directions)
	{
		FHitResult Hit;
		FVector Start = Location;
		FVector End = Start + Dir * Distance;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(IgnoredActor);

		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			if (Hit.Distance < Clearance)
			{
				// För nära hinder
				return false;
			}
		}
	}

	return true;
}
