// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingAI_Controller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "FlyingEnemyAI.h"


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
		
		bool bPlayerVisible = HasLineOfSightToPlayer();
		Blackboard->SetValueAsBool("PlayerVisible", bPlayerVisible);
	}

	if (AFlyingEnemyAI* fiende = Cast<AFlyingEnemyAI>(GetPawn()))
	{
		bool bCanShoot = fiende->CanShoot();
		GetBlackboardComponent()->SetValueAsBool("CanShoot", bCanShoot);
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

bool AFlyingAI_Controller::HasLineOfSightToPlayer() const
{
	APawn* AIPawn = GetPawn();
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!AIPawn || !Player) return false;

	FVector Start = AIPawn->GetActorLocation();
	FVector End = Player->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AIPawn);
	Params.AddIgnoredActor(Player);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1.0f);

	if (!bHit)
	{
		// Ingenting i vägen
		return true;
	}

	// Kållar om den träffar spelaren, alltså om den ser spelaren
	return HitResult.GetActor() == Player;
}

void AFlyingAI_Controller::SetPlayerInRange(float PlayerInRange)
{
	PlayerRangeThreshold = PlayerInRange;
	UE_LOG(LogTemp, Warning, TEXT("PlayerRangeThreshold %f"), PlayerRangeThreshold);
}
