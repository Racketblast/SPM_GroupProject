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
//  Constructor – sets the display name seen in the Behavior‑Tree editor.
// ───────────────────────────────────────────────────────────────────────────── //
UBTTask_ChasePlayer::UBTTask_ChasePlayer(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Name shown on the BT node in the editor.
    NodeName = TEXT("Chase Player");
}

// ───────────────────────────────────────────────────────────────────────────── //
//  ExecuteTask – called by the Behavior‑Tree when this node becomes active.
//
//  @OwnerComp   – BehaviorTreeComponent running the tree.
//  @NodeMemory  – opaque memory block if the task chose to use one (unused).
//  @return      – Succeeded if a move request was issued, Failed otherwise.
// ───────────────────────────────────────────────────────────────────────────── //
EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* )
{
    // 1) grab and Validate we have an AI controller; without it we cannot move.
    AAI_Controller* const AICont = Cast<AAI_Controller>(OwnerComp.GetAIOwner());
    if (!AICont)
    {
        return EBTNodeResult::Failed;   // Controller missing – cannot proceed.
    }

    // 2) grab the blackboard component and checks if retrieved
    UBlackboardComponent* const BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Failed;   // Should never happen – BT guarantees BB.
    }

    const FVector PlayerLocation = BB->GetValueAsVector(GetSelectedBlackboardKey()); //Grabs players location
    if (!PlayerLocation.IsNearlyZero()) // Zero vector = key not set / invalid. Annars kutar AIn mot world origin
    {
        // 3) Issue a simple move request toward that location.
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(AICont, PlayerLocation);

        // 4) Mark the task complete immediately – movement continues in the
        //    background; other tasks can still interrupt this one.
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return EBTNodeResult::Succeeded;
    }

    // PlayerLocation was invalid – fail so the BT can pick another branch.
    return EBTNodeResult::Failed;
}