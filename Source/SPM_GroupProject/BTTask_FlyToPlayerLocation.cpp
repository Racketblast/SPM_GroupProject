#include "BTTask_FlyToPlayerLocation.h"
#include "AIController.h"
#include "FlyingEnemyAI.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyAIUtils.h"
#include "FlyingAIHelper.h" 
#include "PlayerCharacter.h"

UBTTask_FlyToPlayerLocation::UBTTask_FlyToPlayerLocation()
{
	NodeName = TEXT("Fly To Player Location");
	bNotifyTick = true;

	// Sätter default Blackboard key name till MoveToLocation
	MoveToLocationKey.SelectedKeyName = "MoveToLocation";
}

// Lite startlogik 
EBTNodeResult::Type UBTTask_FlyToPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

	AFlyingEnemyAI* FlyingEnemy = Controller ? Cast<AFlyingEnemyAI>(Controller->GetPawn()) : nullptr;
	UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	ACharacter* Player = World ? UGameplayStatics::GetPlayerCharacter(World, 0) : nullptr;

	// Kollar att AI och Pawn existerar
	if (!Controller || !Pawn)
	{
		return EBTNodeResult::Failed;
	}

	// Kallar på en funktion från FlyingAIHelper för att få fram en target location för fienden att åka till
	const FVector FromLocation = FlyingEnemy->GetActorLocation();
	const FVector SmartTarget = FlyingAIHelper::CalculateTargetLocation(
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

	// Sätter SmartTarget (target location) i Blackboards MoveToLocationKey
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(MoveToLocationKey.SelectedKeyName, SmartTarget);

	// Nollställer några variabler. 
	LastLocation = Pawn->GetActorLocation();
	TimeSinceLastMove = 0.f;
	bBackingOff = false;
	BackoffElapsed = 0.f;
	TimeSinceLastTargetUpdate = 0.f;

	return EBTNodeResult::InProgress;
}

// Kör varje frame medan uppgiften är aktiv
void UBTTask_FlyToPlayerLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

	if (!Controller || !Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Hämtar target location och räknar avståndet dit från nuvarande position.
	FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(MoveToLocationKey.SelectedKeyName);
	FVector CurrentLocation = Pawn->GetActorLocation();
	float Distance = FVector::Dist(CurrentLocation, TargetLocation);

	// Kollar om fienden är tillräckligt nära spelaren, och om den är det så avslutas tasken. 
	if (Distance <= AcceptanceRadius)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// kollar om fienden har fastnat, och kallar på CheckIfStuck ifall den har "fastnat"
	// Hoppar över stuck logic ifall fienden redan skjuter mott spelaren
	bool bPlayerInRange = OwnerComp.GetBlackboardComponent()->GetValueAsBool("PlayerInRange");
	bool bPlayerVisible = OwnerComp.GetBlackboardComponent()->GetValueAsBool("PlayerVisible");
	
	TimeSinceLastMove += DeltaSeconds;

	float CurrentDistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);
	float PreviousDistanceToTarget = FVector::Dist(LastLocation, TargetLocation);
	float FrameProgress = PreviousDistanceToTarget - CurrentDistanceToTarget;

	TotalProgressTowardTarget += FrameProgress;
	ProgressCheckTimeAccumulator += DeltaSeconds;
	
	LastLocation = CurrentLocation;

	if (ProgressCheckTimeAccumulator >= StuckCheckInterval)
	{
		if (TotalProgressTowardTarget < MinimumTotalProgressThreshold && Distance > StuckMovementThreshold && !(bPlayerInRange && bPlayerVisible))
		{
			CheckIfStuck(OwnerComp, Pawn, TargetLocation);
		}
		
		TotalProgressTowardTarget = 0.f;
		ProgressCheckTimeAccumulator = 0.f;
	}
	/*if (TimeSinceLastMove >= StuckCheckInterval && Distance > StuckMovementThreshold && !(bPlayerInRange && bPlayerVisible))
	{
		CheckIfStuck(OwnerComp, Pawn, TargetLocation);
		LastLocation = CurrentLocation;
		TimeSinceLastMove = 0.f;
	}*/

	// Ifall den backar tillbaka, väntar den på en cooldown, innan den fortsätter
	// bBackingOff sätts i CheckIfStuck funktionen
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

	// Uppdaterar target location med jämna mellanrum, exempelvis varje sekund
	if (TimeSinceLastTargetUpdate >= TargetUpdateInterval)
	{
		TimeSinceLastTargetUpdate = 0.f;

		AFlyingEnemyAI* FlyingEnemy = Cast<AFlyingEnemyAI>(Pawn);
		UWorld* World = GetWorld();
		ACharacter* Player = World ? UGameplayStatics::GetPlayerCharacter(World, 0) : nullptr;

		if (FlyingEnemy && Player)
		{
			const FVector NewTarget = FlyingAIHelper::CalculateTargetLocation(
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

// Kollar om fienden har fastnat (inte rört sig eller kommit närmare sitt mål), Sedan försöker den att backa undan eller teleportera sig
void UBTTask_FlyToPlayerLocation::CheckIfStuck(UBehaviorTreeComponent& OwnerComp, APawn* Pawn, const FVector& TargetLocation)
{
	// Hoppar över stuck logic ifall fienden redan ser spelaren
	if (OwnerComp.GetBlackboardComponent()->GetValueAsBool("PlayerVisible"))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Enemy is not stuck. It already sees the player"));
		return;
	}

	// MovedDistance = Hur långt fienden faktiskt har rört sig. ProgressTowardGoal = Om fienden har kommit närmare sitt mål.
	const FVector CurrentLocation = Pawn->GetActorLocation();
	const float MovedDistance = FVector::Dist(CurrentLocation, LastLocation);
	const float DistanceToTargetLast = FVector::Dist(LastLocation, TargetLocation);
	const float DistanceToTargetNow = FVector::Dist(CurrentLocation, TargetLocation);
	const float ProgressTowardGoal = DistanceToTargetLast - DistanceToTargetNow;
	
	/*
	UE_LOG(LogTemp, Warning, TEXT("Updating LastLocation: %s"), *CurrentLocation.ToString());
	const FVector BlackboardTarget = OwnerComp.GetBlackboardComponent()->GetValueAsVector(MoveToLocationKey.SelectedKeyName);
	UE_LOG(LogTemp, Warning, TEXT("CheckIfStuck: TargetLocation = %s | BlackboardTarget = %s"), *TargetLocation.ToString(), *BlackboardTarget.ToString());
	UE_LOG(LogTemp, Warning, TEXT("MovedDistance: %.2f | ProgressTowardGoal: %.2f | DistanceToTargetNow: %.2f"), MovedDistance, ProgressTowardGoal, DistanceToTargetNow);
	*/

	// Fienden räknas som fast ifall den knappt rör sig och knappt gör framsteg mott sitt mål som är spelaren.  bBackingOff används för att inte köra denna logik flera gånger i rad.
	if (MovedDistance < StuckMovementThreshold && ProgressTowardGoal < 10.f && !bBackingOff)
	{
		bBackingOff = true;

		if (AFlyingEnemyAI* Enemy = Cast<AFlyingEnemyAI>(Pawn))
		{
			const bool bPlayerSeesEnemy = Enemy->IsVisibleToPlayer();
			//UE_LOG(LogTemp, Warning, TEXT("IfStuck PlayerSeesEnemy: %s"), bPlayerSeesEnemy ? TEXT("true") : TEXT("false"));

			// Om inställningarna tillåter teleportering, och antingen spelaren inte ser fienden, eller det är tillåtet att teleportera även om spelaren ser fienden.
			// Dessa inställningarna sätts i BP för fienden
			if (Enemy->bTeleportIfStuck && (Enemy->bCanTeleportIfVisibleToPlayer || !bPlayerSeesEnemy))
			{
				// Teleport mode
				FVector NewLocation;

				// Använder FindValidTeleportLocation för att hitta en plats att teleportera till
				if (UEnemyAIUtils::FindValidTeleportLocation(Pawn, TargetLocation, NewLocation, Enemy->bAvoidFrontTeleport))
				{
					Pawn->SetActorLocation(NewLocation);
					Enemy->NotifyTeleported();
					UE_LOG(LogTemp, Warning, TEXT("Enemy teleported to escape being stuck."));
				}
			}
			else
			{
				// Backoff fallback, ifall teleportering inte tillåts 

				HandleFallbackManeuver(OwnerComp, CurrentLocation, TargetLocation);
				
				UE_LOG(LogTemp, Warning, TEXT("Flying AI is stuck, backing off"));
				//DrawDebugSphere(GetWorld(), CurrentLocation, 50.f, 12, FColor::Red, false, 1.f);
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

// Den faktiska movementen funktionen, gör själva flygningen mot målet.
void UBTTask_FlyToPlayerLocation::MoveTowardTarget(APawn* Pawn, const FVector& TargetLocation) const
{
	//  Beräknar en riktning mot målet.
	const FVector Direction = (TargetLocation - Pawn->GetActorLocation()).GetSafeNormal();

	// Använder AddMovementInput för att flyga i riktningen, hastigheten kan sättas i BP
	if (AFlyingEnemyAI* Enemy = Cast<AFlyingEnemyAI>(Pawn))
	{
		Enemy->SetCurrentTargetLocation(TargetLocation);
		Enemy->AddMovementInput(Direction, Enemy->FlySpeed * GetWorld()->GetDeltaSeconds());
	}
}

// En fallback som kallas när fienden fastnar.
void UBTTask_FlyToPlayerLocation::HandleFallbackManeuver(UBehaviorTreeComponent& OwnerComp, const FVector& CurrentLocation, const FVector& TargetLocation) const
{
	UWorld* World = GetWorld();
	if (!World) return;

	const float TraceLength = 300.f;
	const FVector ForwardDir = (TargetLocation - CurrentLocation).GetSafeNormal();
	const FVector RightDir = FVector::CrossProduct(ForwardDir, FVector::UpVector);
	const FVector LeftDir = -RightDir;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(FallbackTrace), true);
	FHitResult HitForward, HitLeft, HitRight;

	bool bBlockedForward = World->LineTraceSingleByChannel(HitForward, CurrentLocation, CurrentLocation + ForwardDir * TraceLength, ECC_Visibility, TraceParams);
	bool bBlockedLeft    = World->LineTraceSingleByChannel(HitLeft,    CurrentLocation, CurrentLocation + LeftDir * TraceLength,    ECC_Visibility, TraceParams);
	bool bBlockedRight   = World->LineTraceSingleByChannel(HitRight,   CurrentLocation, CurrentLocation + RightDir * TraceLength,   ECC_Visibility, TraceParams);

	// Draw debug lines
	/*DrawDebugLine(World, CurrentLocation, CurrentLocation + ForwardDir * TraceLength, FColor::Red, false, 1.f, 0, 2.f);
	DrawDebugLine(World, CurrentLocation, CurrentLocation + LeftDir * TraceLength, FColor::Green, false, 1.f, 0, 2.f);
	DrawDebugLine(World, CurrentLocation, CurrentLocation + RightDir * TraceLength, FColor::Blue, false, 1.f, 0, 2.f);*/

	FVector NewDirection = -ForwardDir; // Standard backoff riktningen

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	bool bLastStrafeRight = Blackboard->GetValueAsBool("LastStrafeRight");

	if (bBlockedForward)
	{
		// Försöker med den senaste slumpade riktningen först. 
		if (bLastStrafeRight && !bBlockedRight)
		{
			NewDirection = RightDir;
		}
		else if (!bLastStrafeRight && !bBlockedLeft)
		{
			NewDirection = LeftDir;
		}
		else if (!bBlockedLeft && !bBlockedRight)
		{
			// Slumpar mellan höger och vänster och väljer en riktining mellan dessa, och sedan sparrar den riktningen. 
			bool bGoRight = FMath::RandBool();
			NewDirection = bGoRight ? RightDir : LeftDir;
			Blackboard->SetValueAsBool("LastStrafeRight", bGoRight);
		}
		else if (!bBlockedLeft)
		{
			NewDirection = LeftDir;
			Blackboard->SetValueAsBool("LastStrafeRight", false);
		}
		else if (!bBlockedRight)
		{
			NewDirection = RightDir;
			Blackboard->SetValueAsBool("LastStrafeRight", true);
		}
	}

	const FVector NewTargetLocation = CurrentLocation + NewDirection * BackoffDistance;
	Blackboard->SetValueAsVector(MoveToLocationKey.SelectedKeyName, NewTargetLocation);

	//UE_LOG(LogTemp, Warning, TEXT("Flying AI is stuck, using fallback maneuver"));
	//DrawDebugSphere(World, NewTargetLocation, 50.f, 12, FColor::Purple, false, 1.f);
}

