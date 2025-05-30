// ─────────────────────────────────────────────────────────────────────────────
//  BTTask_FindRandomLocation – Behavior‑Tree Task (FULLY ANNOTATED)
//  ---------------------------------------------------------------------------
//  Purpose
//  -------
//  • Pick a random reachable point within a configurable radius around the AI
//    pawn and write that location into a Blackboard Vector key so a later
//    Move‑To task/service can use it.
//  • Succeeds as soon as a point is found; fails only if any prerequisite
//    object (controller, pawn, nav‑mesh, key) is missing.
//
//  Design Notes
//  ------------
//  * We inherit from **UBTTask_BlackboardBase** so we can expose the target
//    Blackboard key directly in the BT editor and reuse the helper accessors
//    it provides.
//  * The task is intended to be lightweight and non‑blocking: all NavMesh
//    queries are synchronous, so ExecuteTask ends immediately.
//  * SearchRadius is editable per‑instance in the BT (via the Details panel)
//    so different enemy types can wander in larger or smaller areas.
//
//  File layout below:
//    1. Header (class declaration with comments)
//    2. Implementation (.cpp) with step‑by‑step annotations
// ─────────────────────────────────────────────────────────────────────────────

// ========== 1. HEADER =======================================================

#pragma once

#include "CoreMinimal.h"                          // UE core types / macros
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h" // parent class
#include "BehaviorTree/BlackboardComponent.h"         // Blackboard helpers
#include "BTTask_FindRandomLocation.generated.h"       // UHT‑generated include

/**
 * BTTask_FindRandomLocation
 * -------------------------
 * Writes a random NavMesh location to a Blackboard Vector key.
 */
UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FindRandomLocation : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    /** Default constructor allows exposing SearchRadius in the Details panel. */
    explicit UBTTask_FindRandomLocation(const FObjectInitializer& ObjectInitializer);

    /** Core BT hook – runs once when the node is executed in the tree. */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                            uint8* NodeMemory) override;

private:
    /** Max radius (cm) around the pawn to sample for a random point. */
    UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true"))
    float SearchRadius = 1500.f;
};