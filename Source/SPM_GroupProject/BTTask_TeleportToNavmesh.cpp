#include "BTTask_TeleportToNavmesh.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

UBTTask_TeleportToNavmesh::UBTTask_TeleportToNavmesh()
{
    NodeName = TEXT("Teleport To Navmesh");
}

EBTNodeResult::Type UBTTask_TeleportToNavmesh::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;

    FNavLocation NavLocation;
    if (NavSys->GetRandomPointInNavigableRadius(AIPawn->GetActorLocation(), SearchRadius, NavLocation))
    {
        AIPawn->SetActorLocation(NavLocation.Location, false, nullptr, ETeleportType::TeleportPhysics);

        // ✅ Reset the blackboard flag
        if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
        {
            Blackboard->SetValueAsBool(FName("TeleportToNavmesh"), false);
        }

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
