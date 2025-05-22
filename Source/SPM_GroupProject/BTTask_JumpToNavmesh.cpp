#include "BTTask_JumpToNavMesh.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

UBTTask_JumpToNavMesh::UBTTask_JumpToNavMesh()
{
	NodeName = "Jump to Nearest NavMesh Point";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_JumpToNavMesh::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ElapsedTime = 0.0f;

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	CachedCharacter = Cast<ACharacter>(AICon->GetPawn());
	if (!CachedCharacter) return EBTNodeResult::Failed;

	const FVector CurrentLocation = CachedCharacter->GetActorLocation();
	FNavLocation NearestPoint;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;

	if (NavSys->ProjectPointToNavigation(CurrentLocation, NearestPoint, FVector(500, 500, 500)))
	{
		const FVector JumpDirection = (NearestPoint.Location - CurrentLocation).GetSafeNormal();
		FVector LaunchVelocity = JumpDirection * 600.f;
		LaunchVelocity.Z = 500.f; // Vertical boost

		CachedCharacter->LaunchCharacter(LaunchVelocity, true, true);

		// Debug
		DrawDebugLine(GetWorld(), CurrentLocation, NearestPoint.Location, FColor::Green, false, 2.0f, 0, 5.0f);
		DrawDebugSphere(GetWorld(), NearestPoint.Location, 30.f, 12, FColor::Green, false, 2.0f);

		TargetLocation = NearestPoint.Location;
		CachedOwnerComp = &OwnerComp;

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_JumpToNavMesh::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!CachedCharacter || !CachedOwnerComp) return;

	ElapsedTime += DeltaSeconds;

	if (!CachedCharacter->GetCharacterMovement()->IsFalling())
	{
		// Optional: proximity check
		const float Distance = FVector::Dist(CachedCharacter->GetActorLocation(), TargetLocation);
		if (Distance < 200.f) // Relaxed threshold
		{
			if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
			{
				BlackboardComp->SetValueAsBool("TeleportToNavmesh", false);
			}

			FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}

	// Timeout safeguard
	if (ElapsedTime > MaxWaitTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpToNavMesh timed out."));

		if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsBool("TeleportToNavmesh", false);
		}

		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
	}
}
