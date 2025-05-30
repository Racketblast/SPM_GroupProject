#include "BTTask_TeleportToNavmesh.h"
#include "AIController.h"                 // For AAIController
#include "NavigationSystem.h"             // For UNavigationSystemV1 helpers
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"      // So we can safely teleport characters

// ---------------------------------------------------------------------------
// Constructor – give the BT node a nice label.
// ---------------------------------------------------------------------------
UBTTask_TeleportToNavmesh::UBTTask_TeleportToNavmesh()
{
    NodeName = TEXT("Teleport To Navmesh");
}

// ---------------------------------------------------------------------------
// ExecuteTask – single‑frame operation.
// ---------------------------------------------------------------------------
//  1. Validate controller and pawn.
//  2. Ask the nav‑system for a random reachable point within SearchRadius.
//  3. If found, teleport the pawn there using TeleportPhysics to maintain
//     velocity if needed.
//  4. Clear the "TeleportToNavmesh" Blackboard flag so this task doesn’t loop.
// ---------------------------------------------------------------------------
EBTNodeResult::Type UBTTask_TeleportToNavmesh::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* /*NodeMemory – unused because task is non‑latent*/)
{
    // ─── 1. Validate AIController → Pawn  ───────────────────────────────────
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    // ─── 2. Get NavSystem – may be null in title screen / non‑nav worlds. ────
    const UNavigationSystemV1* NavSys =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;

    // ─── 3. Query random point on the NavMesh within SearchRadius. ──────────
    FNavLocation NavLocation;
    if (NavSys->GetRandomPointInNavigableRadius(
            AIPawn->GetActorLocation(),
            SearchRadius,
            NavLocation))
    {
        // TeleportPhysics preserves existing velocity when possible.
        AIPawn->SetActorLocation(
            NavLocation.Location,
            /*bSweep=*/false,
            /*HitResult=*/nullptr,
            ETeleportType::TeleportPhysics);

        // ─── 4. Clear BT flag so other tasks can proceed. ──────────────────
        if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
        {
            Blackboard->SetValueAsBool(FName("TeleportToNavmesh"), false);
        }

        return EBTNodeResult::Succeeded;
    }

    // Couldn’t find a spot – fail so BT can decide what to do next.
    return EBTNodeResult::Failed;
}
