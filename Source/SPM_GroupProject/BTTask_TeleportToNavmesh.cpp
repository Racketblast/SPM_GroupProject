#include "BTTask_TeleportToNavmesh.h"
#include "AIController.h"                 // For AAIController
#include "NavigationSystem.h"             // For UNavigationSystemV1 helpers
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"      // So we can safely teleport characters

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
UBTTask_TeleportToNavmesh::UBTTask_TeleportToNavmesh()
{
    NodeName = TEXT("Teleport To Navmesh");
}

// ---------------------------------------------------------------------------
// ExecuteTask 
// ---------------------------------------------------------------------------
EBTNodeResult::Type UBTTask_TeleportToNavmesh::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8*)
{
    // ─── 1. Hämtar AI controller och pawn, och validate båda  ───────────────────────────────────
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    // ─── 2. Get NavSystem  ────
    const UNavigationSystemV1* NavSys =FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;

    // ─── 3. Find a random nearby NavMesh location within SearchRadius. ──────────
    FNavLocation NavLocation;
    if (NavSys->GetRandomPointInNavigableRadius(AIPawn->GetActorLocation(),SearchRadius,NavLocation))
    {
        // TeleportPhysics preserves existing velocity when possible.
        AIPawn->SetActorLocation(NavLocation.Location,false,nullptr,
            ETeleportType::TeleportPhysics);

        // ─── 4. Clear BT flag ──────────────────
        if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
        {
            //reset blackboard
            Blackboard->SetValueAsBool(FName("TeleportToNavmesh"), false);
        }

        return EBTNodeResult::Succeeded;
    }
    return EBTNodeResult::Failed;
}
