
#include "BTTask_JumpToNavMesh.h"                 
#include "AIController.h"                         
#include "NavigationSystem.h"                     
#include "GameFramework/Character.h"              
#include "GameFramework/CharacterMovementComponent.h" 
#include "BehaviorTree/BlackboardComponent.h"     
#include "DrawDebugHelpers.h"                     
// ────────────────────────────────────────────────
//  Constructor
// ────────────────────────────────────────────────
UBTTask_JumpToNavMesh::UBTTask_JumpToNavMesh()
{
    NodeName   = "Jump to Nearest NavMesh Point"; 
    bNotifyTick = true;                           
}

// ────────────────────────────────────────────────
//  ExecuteTask
// ────────────────────────────────────────────────
EBTNodeResult::Type UBTTask_JumpToNavMesh::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* )
{
    ElapsedTime = 0.0f;                           // resetar timer

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
    
    //Letar närmaste punkten på navmeshen inom en 500 box
    if (NavSys->ProjectPointToNavigation(CurrentLocation,
                                         NearestPoint,
                                         FVector(500, 500, 100)))
    {
        // 3) Build launch vector toward nav point ---------------------------
        const FVector JumpDirection = (NearestPoint.Location - CurrentLocation).GetSafeNormal();

        FVector LaunchVelocity = JumpDirection * 600.f; // horizontal speed
        LaunchVelocity.Z = 100.f;                       // vertical boost

        //Kör Unreals launch
        CachedCharacter->LaunchCharacter(LaunchVelocity,
                                        /*bXYOverride=*/true,
                                        /*bZOverride=*/true);

        // 4) Cache state for TickTask ---------------------------------------------
        TargetLocation  = NearestPoint.Location;  // var vi vill landa 
        CachedOwnerComp = &OwnerComp;             // för FinishLatentTask

        return EBTNodeResult::InProgress;        
    }

    // Projection fail, troligen för långt ifrån navmesh
    return EBTNodeResult::Failed;
}

// ────────────────────────────────────────────────
//  TickTask 
// ────────────────────────────────────────────────
void UBTTask_JumpToNavMesh::TickTask(UBehaviorTreeComponent& OwnerComp,
                                     uint8*, float DeltaSeconds)
{
    if (!CachedCharacter || !CachedOwnerComp) return; 
    
    ElapsedTime += DeltaSeconds;

    // 1) Check if landed ---------------------------------------------------
    if (!CachedCharacter->GetCharacterMovement()->IsFalling())
    {
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
