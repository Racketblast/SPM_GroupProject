// Fill out your copyright notice in the Description page of Project Settings.
#include "BTTask_FindPlayerLocation_Flying.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AIController.h"

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

	FVector TargetLocation = Player->GetActorLocation();
	TargetLocation.Z += ZOffset;
	//TargetLocation.Z = FMath::Min(TargetLocation.Z, AFlyingEnemyAI->MaxAltitude);

	if (bAddRandomOffset)
	{
		FVector RandomOffset(
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius, RandomRadius),
			FMath::FRandRange(-RandomRadius * 0.5f, RandomRadius * 0.5f) 
		);
		TargetLocation += RandomOffset;
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}
