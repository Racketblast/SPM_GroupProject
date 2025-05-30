#pragma once  // Include once per TU – guards against double inclusion

// Engine basics --------------------------------------------------------------
#include "CoreMinimal.h"                                // Core types / macros
#include "BehaviorTree/BTTaskNode.h"                   // Base class
#include "BTTask_StrafeStep.generated.h"               // UHT‑generated glue

// ─────────────────────────────────────────────────────────────────────────────
//  UBTTask_StrafeStep
//  A BT task that makes the AI pawn sidestep (left or right) while facing the
//  player.  It calculates a random candidate point, projects it onto the
//  NavMesh, then issues a MoveTo request.  Success when it reaches the point;
//  failure if LOS is blocked or the move times out.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class SPM_GROUPPROJECT_API UBTTask_StrafeStep : public UBTTaskNode
{
    GENERATED_BODY()  // UCLASS reflection boilerplate

public:
    UBTTask_StrafeStep();  // ctor sets node name + enables Tick

    // Kicks off the MoveTo and returns InProgress
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                            uint8*                  NodeMemory) override;

    // Polls move progress / timeout each tick while in InProgress state
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
                          uint8*                 NodeMemory,
                          float                  DeltaSeconds) override;

protected:  // Tunables exposed to designers in the editor --------------------
    // Ideal sidestep distance (cm)
    UPROPERTY(EditAnywhere, Category = "Strafe")
    float StrafeDistance = 200.f;

    // Random +/- variance applied to StrafeDistance each execution
    UPROPERTY(EditAnywhere, Category = "Strafe")
    float StrafeVariance = 50.f;

    // Distance from target at which we consider the task succeeded (cm)
    UPROPERTY(EditAnywhere, Category = "Strafe")
    float AcceptRadius = 40.f;

    /* How long we allow the MoveTo to run before failing (seconds) */
    UPROPERTY(EditAnywhere, Category = "Strafe")
    float MaxMoveTime = 2.5f;

private:   // Cached state while the task is running --------------------------
    FVector              TargetLocation{ForceInit};   // final MoveTo dest
    class AAIController* CachedController = nullptr;  // convenience ptr
    class ACharacter*    CachedPawn       = nullptr;  // controlled pawn

    bool  bMoveRequested            = false;  // true once we invoked MoveTo
    bool  bSavedOrientRotToMovement = false;  // original movement‑comp flag
    float ElapsedTime               = 0.f;    // timer for MaxMoveTime check
};