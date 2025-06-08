// ───────────────────────────────────────────────────────────────────────────── //
//  BTTask_ChasePlayer – Behavior‑Tree Task  (FULLY ANNOTATED)
//  ---------------------------------------------------------------------------
//  Purpose
//  -------
//  • Reads a Blackboard Vector key that stores the player’s current world
//    position (usually set by a perception or service node).
//  • Commands the AI controller to move its pawn toward that position using
//    `SimpleMoveToLocation`, which wraps `AIController::MoveToLocation`.
//  • Succeeds immediately – the move continues asynchronously; any failure to
//    reach the player is handled by other BT nodes (e.g. an Abort or decorator).
//
//  Key design choices
//  ------------------
//  • The task does **not** wait for the move to finish; that keeps the BT
//    responsive so higher‑priority tasks can interrupt (e.g. "Fire", "Dodge").
//  • We don’t cache the Blackboard key selector in a member variable because
//    UBTTask_BlackboardBase already exposes `GetSelectedBlackboardKey()` which
//    returns the key chosen in the Behavior‑Tree editor.
//  • No input bindings or tick are required – the default character movement
//    handles path‑finding once the destination is set.
// ───────────────────────────────────────────────────────────────────────────── //

#include "BTTask_ChasePlayer.h"

// Project headers -------------------------------------------------------------
#include "AI_Controller.h"                    // Custom AI controller class.

// Unreal headers --------------------------------------------------------------
#include "BehaviorTree/BlackboardComponent.h" // Access Blackboard values.
#include "Blueprint/AIBlueprintHelperLibrary.h" // SimpleMoveToLocation helper.

// ───────────────────────────────────────────────────────────────────────────── //
//  Constructor
// ───────────────────────────────────────────────────────────────────────────── //
UBTTask_ChasePlayer::UBTTask_ChasePlayer(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Name shown on the BT node in the editor.
    NodeName = TEXT("Chase Player");
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* )
{
    // 1) grab and Validate AI controller
    AAI_Controller* const AICont = Cast<AAI_Controller>(OwnerComp.GetAIOwner());
    if (!AICont)
    {
        return EBTNodeResult::Failed;   // Controller saknas
    }

    // 2) grab the blackboard component and checks if retrieved
    UBlackboardComponent* const BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Failed;   
    }

    const FVector PlayerLocation = BB->GetValueAsVector(GetSelectedBlackboardKey()); //Grabs players location
    if (!PlayerLocation.IsNearlyZero()) // Zero vector = key not set / invalid. Annars kutar AIn mot world origin
    {
        // 3) simple move request mot playerlocation
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(AICont, PlayerLocation);

        // 4) Mark the task complete
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return EBTNodeResult::Succeeded;
    }
    
    return EBTNodeResult::Failed;
}