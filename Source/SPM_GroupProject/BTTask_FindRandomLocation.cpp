#include "BTTask_FindRandomLocation.h"               // own header first

#include "AIController.h"                            // standard AIController
#include "AI_Controller.h"                           // custom controller (if any helper funcs are needed later)
#include "NavigationSystem.h"                        // UNavigationSystemV1
#include "NavigationPath.h"                          // only needed for future expansion (e.g. path debug draw)

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
UBTTask_FindRandomLocation::UBTTask_FindRandomLocation(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NodeName = TEXT("Find Random Location In Navmesh");
}

// ---------------------------------------------------------------------------
// ExecuteTask
// ---------------------------------------------------------------------------
EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                           uint8* /*NodeMemory*/)
{
    // 1) Validate we have a controller we expect (derived from AAIController).
    AAI_Controller* const Controller = Cast<AAI_Controller>(OwnerComp.GetAIOwner());
    if (!Controller)
    {
        return EBTNodeResult::Failed;   // no AI Controller → cannot proceed
    }

    // 2) Validate we have a pawn under control.
    APawn* const AIPawn = Controller->GetPawn();
    if (!AIPawn)
    {
        return EBTNodeResult::Failed;   // should never happen, but stay safe
    }

    // 3) Cache origin = pawn position once (cheaper than calling twice).
    const FVector Origin = AIPawn->GetActorLocation();

    // 4) Fetch current NavigationSystem (fails if world has no NavData).
    UNavigationSystemV1* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        return EBTNodeResult::Failed;   // no NavMesh present
    }

    // 5) Ask NavSys for a random reachable point within radius.
    FNavLocation NavLoc;    // holds the result
    const bool bFound = NavSys->GetRandomPointInNavigableRadius(Origin, SearchRadius, NavLoc);

    // 6) If a point was found, push it to the Blackboard.
    if (bFound)
    {
        UBlackboardComponent* const BB = OwnerComp.GetBlackboardComponent();
        if (ensureMsgf(BB, TEXT("BTTask_FindRandomLocation: Blackboard missing!")))
        {
            BB->SetValueAsVector(GetSelectedBlackboardKey(), NavLoc.Location);
        }

        // Notify BT we succeeded (also triggers FinishLatentTask internally).
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return EBTNodeResult::Succeeded;
    }
    
    return EBTNodeResult::Failed;
}
