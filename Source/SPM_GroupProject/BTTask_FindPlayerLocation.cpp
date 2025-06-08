// ───────────────────────────────────────────────────────────────────────────── //
//  BTTask_FindPlayerLocation – Behavior‑Tree Task  (FULLY ANNOTATED)
//  ---------------------------------------------------------------------------
//  Responsibilities
//  ---------------
//  • Query the current player character’s world position.
//  • Convert that raw position into a location guaranteed to be on the NavMesh
//    so a Move‑To task can path there safely.
//      – If **SearchRandom** is *true* (tunable in the Blueprint inspector),
//        pick a random reachable point within **SearchRadius** meters of the
//        player.  This is great for strafing / circling behaviours.
//      – Otherwise, snap the exact player location to the closest navigable
//        point (projects through walls/floors if needed).
//  • Store that vector into the Blackboard key chosen in the BT editor.
//  • Return **Succeeded** when a valid NavMesh point is written; **Failed**
//    otherwise so the BT can try an alternate branch.
//
//  Why do the NavMesh checks here instead of in the MoveTo node?
//  ------------------------------------------------------------
//  MoveTo will abort immediately if given a non‑navigable destination, which
//  creates a flicker of “Fail → Replan” spam when the player stands outside
//  the agent’s NavMesh (ledge, air, etc.).  By projecting up‑front we avoid
//  that and get predictable behaviour.
// ───────────────────────────────────────────────────────────────────────────── //

#include "BTTask_FindPlayerLocation.h"

// Engine headers
#include "NavigationSystem.h"                       // NavMesh queries
#include "BehaviorTree/BlackboardComponent.h"       // Blackboard access helpers
#include "GameFramework/Character.h"                // ACharacter base class
#include "Kismet/GameplayStatics.h"                 // GetPlayerCharacter helper

// ────────────────────────────────────────────────────────────────
// Constructor
UBTTask_FindPlayerLocation::UBTTask_FindPlayerLocation(
    const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
    NodeName = TEXT("Find Player Location");
}

// ────────────────────────────────────────────────────────────────
// ExecuteTask
EBTNodeResult::Type UBTTask_FindPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* )
{
    // 1) Fetch the player pawn
    if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        const FVector PlayerLocation = Player->GetActorLocation();

        // 2) be navsystem om en plats den faktiskt kan gå till
        if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
        {
            FNavLocation ProjectedLocation; 


            //Hittils oanvänd logik för att få en fiende att sprida ut sig lite mer så inte alla springer i en kö
            //Används endast om Findplayerlocation har icheckat SearchRandom inne i Behavior tree
            if (SearchRandom)
            {
                // 2a) Pick a random reachable point around the player
                if (NavSys->GetRandomPointInNavigableRadius(PlayerLocation,SearchRadius,ProjectedLocation))  
                {
                    OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(),
                                                        ProjectedLocation.Location);
                    
                    FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                    return EBTNodeResult::Succeeded;
                }
            }
            //Logiken för närmaste navmesh punkt
            else
            {
                // 2b) Snappa spelarens location till närmaste navmesh punkt
                //     skapa box runt spelaren med 100,100,1500
                static const FVector Extent(100.f, 100.f, 1500.f);

                //Hitta närmaste navmesh punkt inom boxen och storea det i projectedlocation
                if (NavSys->ProjectPointToNavigation(PlayerLocation,ProjectedLocation,Extent))
                {
                    //Skriver den nya valid projectedlocation till blackboarden så den kan användas i nästa Moveto i chase
                    OwnerComp.GetBlackboardComponent()
                             ->SetValueAsVector(GetSelectedBlackboardKey(),
                                                ProjectedLocation.Location);

                    FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                    return EBTNodeResult::Succeeded;
                }
            }
        }
    }
    return EBTNodeResult::Failed;
}