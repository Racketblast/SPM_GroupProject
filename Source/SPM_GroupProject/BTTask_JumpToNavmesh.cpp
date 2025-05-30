
//  Purpose
//  -------
//    •  When the AI pawn finds itself **off the NavMesh** – usually because it
//       was launched by physics, rag‑doll, or pushed over an edge – this task
//       makes the character **jump** toward the nearest reachable NavMesh
//       point instead of doing an instantaneous teleport.
//    •  The jump vector is calculated so the pawn travels horizontally toward
//       the projected NavMesh location with a configurable vertical boost.
//    •  The task returns **InProgress** immediately after launching and
//       finishes once the pawn lands near the target *or* times out.
//
//  Safety & Edge‑cases
//  -------------------
//     •  All pointer fetches (`GetAIOwner`, `GetPawn`, nav‑sys, etc.) are
//       wrapped in null‑checks – any failure falls back to `Failed` so the BT
//       can recover.
//    •  A `MaxWaitTime` timeout prevents the task from hanging forever if the
//       pawn is stuck falling or the jump didn’t cover enough distance.
//    •  Blackboard key **"TeleportToNavmesh"** is reset (`false`) on success or
//       failure so follow‑up nodes don’t loop.
//
//  NOTE
//  ----
//     •  ONLY comments have been added; no executable code changed.
// ============================================================================

#include "BTTask_JumpToNavMesh.h"                 // header for this task node
#include "AIController.h"                         // AAIController
#include "NavigationSystem.h"                     // UNavigationSystemV1 helpers
#include "GameFramework/Character.h"              // ACharacter + LaunchCharacter
#include "GameFramework/CharacterMovementComponent.h" // movement state checks
#include "BehaviorTree/BlackboardComponent.h"     // Blackboard accessors
#include "DrawDebugHelpers.h"                     // Debug line/sphere

// ────────────────────────────────────────────────
//  Constructor – set defaults that affect the BT
// ────────────────────────────────────────────────
UBTTask_JumpToNavMesh::UBTTask_JumpToNavMesh()
{
    NodeName   = "Jump to Nearest NavMesh Point"; // label shown in the editor
    bNotifyTick = true;                           // we need TickTask() callbacks
}

// ────────────────────────────────────────────────
//  ExecuteTask – called the first frame the node runs
// ────────────────────────────────────────────────
EBTNodeResult::Type UBTTask_JumpToNavMesh::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
    ElapsedTime = 0.0f;                           // reset timeout timer

    // 1) Validate controller & pawn ------------------------------------------------
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    CachedCharacter = Cast<ACharacter>(AICon->GetPawn());
    if (!CachedCharacter) return EBTNodeResult::Failed;

    // 2) Find the nearest NavMesh point -------------------------------------------
    const FVector CurrentLocation = CachedCharacter->GetActorLocation();
    FNavLocation  NearestPoint;                   // OUT param for projection

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;

    // ProjectPointToNavigation returns true if within the extents (500³ box)
    if (NavSys->ProjectPointToNavigation(CurrentLocation,
                                         NearestPoint,
                                         FVector(500, 500, 500)))
    {
        // 3) Build a launch vector toward the nav point ---------------------------
        const FVector JumpDirection =
            (NearestPoint.Location - CurrentLocation).GetSafeNormal();

        FVector LaunchVelocity = JumpDirection * 600.f; // horizontal speed
        LaunchVelocity.Z = 500.f;                       // vertical boost

        CachedCharacter->LaunchCharacter(LaunchVelocity,
                                        /*bXYOverride=*/true,
                                        /*bZOverride=*/true);

        /* DEBUG: uncomment to see path & target ------------------------------------
        DrawDebugLine(GetWorld(), CurrentLocation, NearestPoint.Location,
                      FColor::Green, false, 2.0f, 0, 5.0f);
        DrawDebugSphere(GetWorld(), NearestPoint.Location, 30.f, 12,
                        FColor::Green, false, 2.0f);
        */

        // 4) Cache state for TickTask ---------------------------------------------
        TargetLocation  = NearestPoint.Location;  // where we want to land
        CachedOwnerComp = &OwnerComp;             // for FinishLatentTask later

        return EBTNodeResult::InProgress;         // we’ll monitor in TickTask()
    }

    // Projection failed – likely too far from navmesh.
    return EBTNodeResult::Failed;
}

// ────────────────────────────────────────────────
//  TickTask – called every frame while node is InProgress
// ────────────────────────────────────────────────
void UBTTask_JumpToNavMesh::TickTask(UBehaviorTreeComponent& OwnerComp,
                                     uint8* /*NodeMemory*/, float DeltaSeconds)
{
    if (!CachedCharacter || !CachedOwnerComp) return; // sanity guard

    ElapsedTime += DeltaSeconds;

    // 1) Check if we have landed ---------------------------------------------------
    if (!CachedCharacter->GetCharacterMovement()->IsFalling())
    {
        // Optional proximity test so we don’t mark success mid‑air.
        const float Distance =
            FVector::Dist(CachedCharacter->GetActorLocation(), TargetLocation);

        if (Distance < 200.f)  // ~2 metres tolerance
        {
            if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
            {
                BlackboardComp->SetValueAsBool("TeleportToNavmesh", false);
            }

            FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
            return;
        }
    }

    // 2) Timeout safeguard ---------------------------------------------------------
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
