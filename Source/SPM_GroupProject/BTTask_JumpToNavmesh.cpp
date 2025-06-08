
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

    // 2) Get ActorLocation -------------------------------------------
    const FVector CurrentLocation = CachedCharacter->GetActorLocation();
    FNavLocation  NearestPoint; // param för projection, sets senare

    //Hämtar navsystem
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;
    
    //Letar närmaste punkten på navmeshen inom en 500³ box
    if (NavSys->ProjectPointToNavigation(CurrentLocation,
                                         NearestPoint,
                                         FVector(500, 500, 100)))
    {
        // 3) Build a launch vector toward the nav point ---------------------------
        const FVector JumpDirection = (NearestPoint.Location - CurrentLocation).GetSafeNormal();

        FVector LaunchVelocity = JumpDirection * 600.f; // horizontal speed
        LaunchVelocity.Z = 100.f;                       // vertical boost

        //Kör Unreals launch
        CachedCharacter->LaunchCharacter(LaunchVelocity,
                                        /*bXYOverride=*/true,
                                        /*bZOverride=*/true);

         /*DEBUG: ------------------------------------*/
        DrawDebugLine(GetWorld(), CurrentLocation, NearestPoint.Location,
                      FColor::Green, false, 2.0f, 0, 5.0f);
        DrawDebugSphere(GetWorld(), NearestPoint.Location, 30.f, 12,
                        FColor::Green, false, 2.0f);
        /**/

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
                                     uint8*, float DeltaSeconds)
{
    if (!CachedCharacter || !CachedOwnerComp) return; // sanity guard

    //Update elapsed time
    ElapsedTime += DeltaSeconds;

    // 1) Check if landed ---------------------------------------------------
    if (!CachedCharacter->GetCharacterMovement()->IsFalling())
    {
        // Optional proximity test so we don’t mark success mid‑air.
        const float Distance =
            FVector::Dist(CachedCharacter->GetActorLocation(), TargetLocation);

        if (Distance < 200.f)  // ~2 metres tolerance
        {
            if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
            {
                //reset blackboard flag
                BlackboardComp->SetValueAsBool("TeleportToNavmesh", false);
            }

            FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
            return;
        }
    }

    // 2) Timeout safeguard, antar fail om elapsed time för långt---------------------------------------------------------
    if (ElapsedTime > MaxWaitTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("JumpToNavMesh timed out."));

        if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
        {
            //reset blackboard flag
            BlackboardComp->SetValueAsBool("TeleportToNavmesh", false);
        }

        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
    }
}
