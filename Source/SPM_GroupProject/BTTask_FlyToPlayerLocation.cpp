#include "BTTask_FlyToPlayerLocation.h"
#include "AIController.h"
#include "FlyingEnemyAI.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyAIUtils.h"
#include "FlyingAIHelper.h" 

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

	AFlyingEnemyAI* FlyingEnemy = Controller ? Cast<AFlyingEnemyAI>(Controller->GetPawn()) : nullptr;
	UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	ACharacter* Player = World ? UGameplayStatics::GetPlayerCharacter(World, 0) : nullptr;
	
	if (!Controller || !Pawn)
	{
		return EBTNodeResult::Failed;
	}

	
	const FVector FromLocation = FlyingEnemy->GetActorLocation();
	const FVector SmartTarget = FlyingAIHelper::ComputeSmartTargetLocation(
		World,
		Player,
		FlyingEnemy,
		FromLocation,
		FlyingEnemy->ZOffset,
		FlyingEnemy->bAddRandomOffset,
		FlyingEnemy->RandomRadius,
		FlyingEnemy->ObstacleCheckDistance,
		FlyingEnemy->ObstacleClearance
	);
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(MoveToLocationKey.SelectedKeyName, SmartTarget);

	

	LastLocation = Pawn->GetActorLocation();
	TimeSinceLastMove = 0.f;
	bBackingOff = false;
	BackoffElapsed = 0.f;

	TimeSinceLastTargetUpdate = 0.f;

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
	bool bPlayerInRange = OwnerComp.GetBlackboardComponent()->GetValueAsBool("PlayerInRange");
	bool bPlayerVisible = OwnerComp.GetBlackboardComponent()->GetValueAsBool("PlayerVisible");

	bool bTargetRecentlyChanged = TimeSinceLastTargetUpdate < 0.1f;

	// Hoppar över stuck logic ifall fienden redan skjuter mott spelaren
	TimeSinceLastMove += DeltaSeconds;
	if (!bTargetRecentlyChanged && TimeSinceLastMove >= StuckCheckInterval && Distance > StuckMovementThreshold * 2.f && !(bPlayerInRange && bPlayerVisible))
	{
		CheckIfStuck(OwnerComp, Pawn, TargetLocation);
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
		bBackingOff = false;
		BackoffElapsed = 0.f;
	}

	TimeSinceLastTargetUpdate += DeltaSeconds;

	if (TimeSinceLastTargetUpdate >= TargetUpdateInterval)
	{
		TimeSinceLastTargetUpdate = 0.f;

		AFlyingEnemyAI* FlyingEnemy = Cast<AFlyingEnemyAI>(Pawn);
		UWorld* World = GetWorld();
		ACharacter* Player = World ? UGameplayStatics::GetPlayerCharacter(World, 0) : nullptr;

		if (FlyingEnemy && Player)
		{
			const FVector NewTarget = FlyingAIHelper::ComputeSmartTargetLocation(
				World,
				Player,
				FlyingEnemy,
				Pawn->GetActorLocation(),
				FlyingEnemy->ZOffset,
				FlyingEnemy->bAddRandomOffset,
				FlyingEnemy->RandomRadius,
				FlyingEnemy->ObstacleCheckDistance,
				FlyingEnemy->ObstacleClearance
			);
			
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(MoveToLocationKey.SelectedKeyName, NewTarget);
			//UE_LOG(LogTemp, Warning, TEXT("Updating Flying AI TargetLocation"));
		}
	}

	// Den faktiska movementen
	MoveTowardTarget(Pawn, TargetLocation);
}


void UBTTask_FlyToPlayerLocation::CheckIfStuck(UBehaviorTreeComponent& OwnerComp, APawn* Pawn, const FVector& TargetLocation)
{
	//bool InRange = OwnerComp.GetBlackboardComponent()->GetValueAsBool("PlayerInRange");
	bool HasLineOfSight = OwnerComp.GetBlackboardComponent()->GetValueAsBool("PlayerVisible");
	
	// Hoppar över stuck logic ifall fienden redan ser spelaren
	if (HasLineOfSight)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy is not stuck. It already sees the player"));
		return;
	}
	
	const FVector CurrentLocation = Pawn->GetActorLocation();
	const float MovedDistance = FVector::Dist(CurrentLocation, LastLocation);
	const float DistanceToTargetLast = FVector::Dist(LastLocation, TargetLocation);
	const float DistanceToTargetNow = FVector::Dist(CurrentLocation, TargetLocation);
	const float ProgressTowardGoal = DistanceToTargetLast - DistanceToTargetNow;
	
	//UE_LOG(LogTemp, Warning, TEXT("Updating LastLocation: %s"), *CurrentLocation.ToString());



	const FVector BlackboardTarget = OwnerComp.GetBlackboardComponent()->GetValueAsVector(MoveToLocationKey.SelectedKeyName);
	/*UE_LOG(LogTemp, Warning, TEXT("CheckIfStuck: TargetLocation = %s | BlackboardTarget = %s"), *TargetLocation.ToString(), *BlackboardTarget.ToString());

	UE_LOG(LogTemp, Warning, TEXT("MovedDistance: %.2f | ProgressTowardGoal: %.2f | DistanceToTargetNow: %.2f"), 
	MovedDistance, ProgressTowardGoal, DistanceToTargetNow);*/

	// Fienden räknas som fast ifall den knappt rör sig och knappt gör framsteg mott sitt mål som är spelaren. 
	if (MovedDistance < StuckMovementThreshold && ProgressTowardGoal < 10.f && !bBackingOff)
	{
		bBackingOff = true;

		if (AFlyingEnemyAI* Enemy = Cast<AFlyingEnemyAI>(Pawn))
		{
			const bool bPlayerSeesEnemy = Enemy->IsVisibleToPlayer();
			//UE_LOG(LogTemp, Warning, TEXT("IfStuck PlayerSeesEnemy: %s"), bPlayerSeesEnemy ? TEXT("true") : TEXT("false"));

			if (Enemy->bTeleportIfStuck && (Enemy->bCanTeleportIfVisibleToPlayer || !bPlayerSeesEnemy))
			{
				// Teleport mode
				FVector NewLocation;
				if (UEnemyAIUtils::FindValidTeleportLocation(Pawn, TargetLocation, NewLocation, Enemy->bAvoidFrontTeleport))
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
				FVector NewTargetLocation = CurrentLocation + Backward;

				OwnerComp.GetBlackboardComponent()->SetValueAsVector(MoveToLocationKey.SelectedKeyName, NewTargetLocation);
				UE_LOG(LogTemp, Warning, TEXT("Flying AI is stuck, backing off"));
				DrawDebugSphere(GetWorld(), CurrentLocation, 50.f, 12, FColor::Red, false, 1.f);
			}
		}
	}
	else if (bBackingOff)
	{
		// Inte längre fast
		bBackingOff = false;
	}
	
	LastLocation = CurrentLocation;
}

// Den faktiska movementen funktionen
void UBTTask_FlyToPlayerLocation::MoveTowardTarget(APawn* Pawn, const FVector& TargetLocation)
{
	const FVector Direction = (TargetLocation - Pawn->GetActorLocation()).GetSafeNormal();

	if (AFlyingEnemyAI* Enemy = Cast<AFlyingEnemyAI>(Pawn))
	{
		Enemy->SetCurrentTargetLocation(TargetLocation);
		Enemy->AddMovementInput(Direction, Enemy->FlySpeed * GetWorld()->GetDeltaSeconds());
	}
}