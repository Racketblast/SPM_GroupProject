// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingAI_Controller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "FlyingEnemyAI.h"

/*AFlyingAI_Controller::AFlyingAI_Controller()
{
	PrimaryActorTick.bCanEverTick = true;
}*/

void AFlyingAI_Controller::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Blackboard) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	APawn* ControlledPawn = GetPawn();

	if (Player && ControlledPawn)
	{
		float Distance = FVector::Dist(Player->GetActorLocation(), ControlledPawn->GetActorLocation());
		bool bIsInRange = Distance <= PlayerRangeThreshold;
		Blackboard->SetValueAsBool("PlayerInRange", bIsInRange);
	}
}

void AFlyingAI_Controller::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AFlyingEnemyAI* FlyingAI = Cast<AFlyingEnemyAI>(InPawn))
	{
		if (UBehaviorTree* BehaviorTree = FlyingAI->GetBehaviorTree())
		{
			UBlackboardComponent* b;
			UseBlackboard(BehaviorTree->BlackboardAsset, b);
			Blackboard = b;
			RunBehaviorTree(BehaviorTree);
		}
	}
}