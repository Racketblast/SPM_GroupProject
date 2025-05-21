#include "BTTask_JumpToNavMesh.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_JumpToNavMesh::UBTTask_JumpToNavMesh()
{
	NodeName = "Jump to Nearest NavMesh Point";
}

EBTNodeResult::Type UBTTask_JumpToNavMesh::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	ACharacter* Character = Cast<ACharacter>(AICon->GetPawn());
	if (!Character) return EBTNodeResult::Failed;

	const FVector CurrentLocation = Character->GetActorLocation();
	FNavLocation NearestPoint;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;

	if (NavSys->ProjectPointToNavigation(CurrentLocation, NearestPoint, FVector(500, 500, 500)))
	{
		const FVector JumpDirection = (NearestPoint.Location - CurrentLocation).GetSafeNormal();
		FVector LaunchVelocity = JumpDirection * 600.f;
		LaunchVelocity.Z = 500.f;

		Character->LaunchCharacter(LaunchVelocity, true, true);

		//  Reset blackboard key
		if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsBool("TeleportToNavmesh", false);
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
