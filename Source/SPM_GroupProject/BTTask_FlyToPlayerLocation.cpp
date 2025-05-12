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
		if (MovedDistance < StuckMovementThreshold)
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

bool UBTTask_FlyToPlayerLocation::FindValidTeleportLocation(APawn* Pawn, FVector TargetLocation, FVector& OutLocation)
{
	AActor* Player = UGameplayStatics::GetPlayerPawn(Pawn->GetWorld(), 0);
	if (!Player) return false;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector PlayerForward = Player->GetActorForwardVector();

	auto TryFindLocation = [&](bool bAvoidFront) -> bool
	{
		for (int32 i = 0; i < 20; ++i)
		{
			float AngleDegrees = FMath::RandRange(0.f, 360.f);
			float Angle = FMath::DegreesToRadians(AngleDegrees);
			float Radius = FMath::RandRange(200.f, 600.f);
			FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

			FVector TestLocation = PlayerLocation + Offset;
			TestLocation.Z += 200.f;

			FVector ToTestLocation = (TestLocation - PlayerLocation).GetSafeNormal();
			float Dot = FVector::DotProduct(PlayerForward, ToTestLocation);

			bool bLineClear = !Pawn->GetWorld()->LineTraceTestByChannel(TestLocation, PlayerLocation, ECC_Visibility);
			bool bIsFlyable = IsFlyableLocation(Pawn, Pawn->GetWorld(), TestLocation, 50.f);

			if ((Dot < 0.5f || !bAvoidFront) && bLineClear && bIsFlyable)
			{
				OutLocation = TestLocation;
				DrawDebugSphere(Pawn->GetWorld(), TestLocation, 50.f, 12, bAvoidFront ? FColor::Cyan : FColor::Green, false, 2.0f);
				return true;
			}
		}
		return false;
	};

	// Första försöket, undvik att teleportera framför spelaren, så att den ser teleporteringen
	if (TryFindLocation(true))
	{
		return true;
	}

	// Om det första försöket misslyckas, så körs ett annat där fienden får teleportera til spelarens field of view så att dem ser teleporteringen
	return TryFindLocation(false);
}


bool UBTTask_FlyToPlayerLocation::IsFlyableLocation(APawn* Pawn, UWorld* World, FVector Location, float ClearanceRadius)
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn); 
	Params.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(World, 0)); 

	// Kållar om stället är tomt, så att inga hinder är ivägen
	return !World->OverlapBlockingTestByChannel(
		Location,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ClearanceRadius),
		Params
	);
}
