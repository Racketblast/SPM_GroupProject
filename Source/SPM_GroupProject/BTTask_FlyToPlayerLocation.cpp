#include "BTTask_FlyToPlayerLocation.h"
#include "AIController.h"
#include "FlyingEnemyAI.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

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
	if (!Controller || !Controller->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	LastLocation = Controller->GetPawn()->GetActorLocation();
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
		if (MovedDistance < StuckMovementThreshold)
		{
			// Ifall den fastnar, så använder den sig av en backoff, alltså typ en failsafe. 
			if (!bBackingOff)
			{
				bBackingOff = true;
				BackoffElapsed = 0.0f;

				// rör sig bakåt och uppdaterar MoveToLocationKey
				FVector Backward = -(TargetLocation - CurrentLocation).GetSafeNormal() * BackoffDistance;
				FVector NewBackoffTarget = CurrentLocation + Backward;
				OwnerComp.GetBlackboardComponent()->SetValueAsVector(MoveToLocationKey.SelectedKeyName, NewBackoffTarget);

				UE_LOG(LogTemp, Warning, TEXT("Flying AI is stuck, backing off from %s"), *CurrentLocation.ToString());
				/*DrawDebugSphere(GetWorld(), CurrentLocation, 50.f, 12, FColor::Red, false, 1.f);
				DrawDebugLine(GetWorld(), CurrentLocation, NewBackoffTarget, FColor::Yellow, false, 1.f, 0, 2.f);
				DrawDebugBox(GetWorld(), LastLocation, FVector(5,5,5), FColor::Blue, false, 1.f);*/
			}
		}
		else
		{
			bBackingOff = false;
		}
		LastLocation = CurrentLocation;
		TimeSinceLastMove = 0.f;
	}

	// Ifall den backartillbaka, väntar den på en cooldown, innan den fortsätter
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
		Enemy->AddMovementInput(Direction, 1.0f);
	}
}

