// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RepositionFlyingEnemy.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"

UBTTask_RepositionFlyingEnemy::UBTTask_RepositionFlyingEnemy()
{
	NodeName = "Reposition Flying Enemy";
}

EBTNodeResult::Type UBTTask_RepositionFlyingEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player)
	{
		return EBTNodeResult::Failed;
	}

	FVector PlayerLoc = Player->GetActorLocation();
	
	// Väljer en random riktning och distans innom RepositionRadius.
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(200.f, RepositionRadius);
	RandomOffset.Z = FMath::FRandRange(ZOffsetMin, ZOffsetMax);

	FVector TargetLocation = PlayerLoc + RandomOffset;

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(TargetLocation);
	MoveReq.SetAcceptanceRadius(50.f);
	MoveReq.SetUsePathfinding(false); 
	MoveReq.SetCanStrafe(true);

	FNavPathSharedPtr PathPtr;
	if (AIController->MoveTo(MoveReq, &PathPtr) == EPathFollowingRequestResult::RequestSuccessful)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}