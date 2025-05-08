#include "BTTask_FlyToPlayerLocation.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
			// ifall den fastnar, så använder den sig av en backoff, alltså typ en failsafe. 
			if (!bBackingOff)
			{
				bBackingOff = true;
				FVector Backward = -(TargetLocation - CurrentLocation).GetSafeNormal() * BackoffDistance;
				TargetLocation = CurrentLocation + Backward;
				OwnerComp.GetBlackboardComponent()->SetValueAsVector(MoveToLocationKey.SelectedKeyName, TargetLocation);

				UE_LOG(LogTemp, Warning, TEXT("Flying AI is stuck, backing off"));
				DrawDebugSphere(GetWorld(), CurrentLocation, 50.f, 12, FColor::Red, false, 1.f);
			}
		}
		else
		{
			bBackingOff = false;
		}
		LastLocation = CurrentLocation;
		TimeSinceLastMove = 0.f;
	}
	
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	Pawn->AddMovementInput(Direction, 1.0f, false);
}


