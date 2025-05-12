#include "BTTask_FlyToPlayerLocation.h"
#include "AIController.h"
#include "FlyingEnemyAI.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyAIUtils.h"

UBTTask_FlyToPlayerLocation::UBTTask_FlyToPlayerLocation()
{
	NodeName = TEXT("Fly To Player Location");
	bNotifyTick = true;

	// Sätter default Blackboard key name till MoveToLocation
	MoveToLocationKey.SelectedKeyName = "MoveToLocation";
}

EBTNodeResult::Type UBTTask_FlyToPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (!Controller || !Pawn)
	{
		return EBTNodeResult::Failed;
	}

	LastLocation = Pawn->GetActorLocation();
	TimeSinceLastMove = 0.f;
	bBackingOff = false;
	BackoffElapsed = 0.f;

	return EBTNodeResult::InProgress;
}

void UBTTask_FlyToPlayerLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

	if (!Controller || !Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(MoveToLocationKey.SelectedKeyName);
	FVector CurrentLocation = Pawn->GetActorLocation();
	float Distance = FVector::Dist(CurrentLocation, TargetLocation);

	// Kollar om fienden är tillräckligt nära spelaren. 
	if (Distance <= AcceptanceRadius)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// kollar om fienden har fastnat
	TimeSinceLastMove += DeltaSeconds;
	if (TimeSinceLastMove >= StuckCheckInterval)
	{
		float MovedDistance = FVector::Dist(CurrentLocation, LastLocation);
		float DistanceToTargetLast = FVector::Dist(LastLocation, TargetLocation);
		float DistanceToTargetNow = FVector::Dist(CurrentLocation, TargetLocation);
		float ProgressTowardGoal = DistanceToTargetLast - DistanceToTargetNow;

		// Fienden räknas som fast ifall den knappt rör sig och knappt gör framsteg mott sitt mål som är spelaren. 
		if (MovedDistance < StuckMovementThreshold && ProgressTowardGoal < 10.f)
		{
			if (!bBackingOff)
			{
				bBackingOff = true;

				if (AFlyingEnemyAI* Enemy = Cast<AFlyingEnemyAI>(Pawn))
				{
					if (Enemy->bTeleportIfStuck)
					{
						// Teleport mode
						FVector NewLocation;
						if (UEnemyAIUtils::FindValidTeleportLocation(Pawn, TargetLocation, NewLocation))
						{
							Pawn->SetActorLocation(NewLocation);
							Enemy->NotifyTeleported();
							UE_LOG(LogTemp, Warning, TEXT("Enemy teleported to escape being stuck."));
						}
					}
					else
					{
						// Backoff fallback
						FVector Backward = -(TargetLocation - CurrentLocation).GetSafeNormal() * BackoffDistance;
						TargetLocation = CurrentLocation + Backward;
						OwnerComp.GetBlackboardComponent()->SetValueAsVector(MoveToLocationKey.SelectedKeyName, TargetLocation);

						UE_LOG(LogTemp, Warning, TEXT("Flying AI is stuck, backing off"));
						DrawDebugSphere(GetWorld(), CurrentLocation, 50.f, 12, FColor::Red, false, 1.f);
					}
				}
			}
		}
		else
		{
			bBackingOff = false;
		}
		LastLocation = CurrentLocation;
		TimeSinceLastMove = 0.f;
	}

	// Ifall den backar tillbaka, väntar den på en cooldown, innan den fortsätter
	if (bBackingOff)
	{
		BackoffElapsed += DeltaSeconds;
		if (BackoffElapsed < BackoffCooldownTime)
		{
			return;
		}
		else
		{
			bBackingOff = false;
			BackoffElapsed = 0.f;
		}
	}

	// Den faktiska movmenten
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	if (AFlyingEnemyAI* Enemy = Cast<AFlyingEnemyAI>(Pawn))
	{
		Enemy->SetCurrentTargetLocation(TargetLocation);
		Enemy->AddMovementInput(Direction, Enemy->FlySpeed * GetWorld()->GetDeltaSeconds());
	}
}
