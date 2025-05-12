#include "BTTask_FlyToPlayerLocation.h"
#include "AIController.h"
#include "FlyingEnemyAI.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

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
			if (!bBackingOff)
			{
				bBackingOff = true;

				AFlyingEnemyAI* Enemy = Cast<AFlyingEnemyAI>(Pawn);
				if (Enemy && Enemy->bTeleportIfStuck)
				{
					// Teleport mode
					FVector NewLocation;
					if (FindValidTeleportLocation(Pawn, TargetLocation, NewLocation))
					{
						Pawn->SetActorLocation(NewLocation, false);
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
					/*DrawDebugSphere(GetWorld(), CurrentLocation, 50.f, 12, FColor::Red, false, 1.f);
					DrawDebugLine(GetWorld(), CurrentLocation, NewBackoffTarget, FColor::Yellow, false, 1.f, 0, 2.f);
					DrawDebugBox(GetWorld(), LastLocation, FVector(5,5,5), FColor::Blue, false, 1.f);*/
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
		Enemy->AddMovementInput(Direction, Enemy->FlySpeed * GetWorld()->GetDeltaSeconds());
	}
}

bool UBTTask_FlyToPlayerLocation::FindValidTeleportLocation(APawn* Pawn, FVector TargetLocation, FVector& OutLocation)
{
	AActor* Player = UGameplayStatics::GetPlayerPawn(Pawn->GetWorld(), 0);
	if (!Player) return false;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector PlayerForward = Player->GetActorForwardVector();

	for (int32 i = 0; i < 20; ++i)
	{
		float Angle = FMath::RandRange(0.f, 360.f);
		float Radius = FMath::RandRange(200.f, 600.f);
		FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

		FVector TestLocation = PlayerLocation + Offset;
		TestLocation.Z += 200.f; // Hover above

		FVector ToTestLocation = (TestLocation - PlayerLocation).GetSafeNormal();
		float Dot = FVector::DotProduct(PlayerForward, ToTestLocation);

		// Only accept positions *not* in front of the player (Dot < 0.5)
		if (Dot < 0.5f && Pawn->GetWorld()->LineTraceTestByChannel(TestLocation, PlayerLocation, ECC_Visibility) == false && IsFlyableLocation(Pawn->GetWorld(), TestLocation, 100.f))
		{
			OutLocation = TestLocation;
			DrawDebugSphere(Pawn->GetWorld(), TestLocation, 50.f, 12, FColor::Cyan, false, 2.0f);
			return true;
		}
	}

	return false;
}


bool UBTTask_FlyToPlayerLocation::IsFlyableLocation(UWorld* World, FVector Location, float ClearanceRadius)
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(World, 0));

	// Sphere trace to make sure space is clear
	return !World->OverlapBlockingTestByChannel(
		Location,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ClearanceRadius),
		Params
	);
}