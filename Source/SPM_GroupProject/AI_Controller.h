// ───────────────────────────────────────────────────────────────────────────── //
//  AI_Controller – Custom AIController class (FULLY ANNOTATED)
//  ---------------------------------------------------------------------------
//  This controller glues together three core systems for an enemy pawn:
//    • Behaviour Tree (BT) / Blackboard – sets up and runs the tree supplied by
//      the pawn.
//    • Perception – uses a UAIPerceptionComponent with sight to update the
//      Blackboard key "CanSeePlayerCharacter" whenever the player is (un)seen.
//    • Navigation – enables CrowdFollowing for flock‑like obstacle avoidance.
// ───────────────────────────────────────────────────────────────────────────── //

#pragma once

#include "CoreMinimal.h"                       // Engine core types / utilities
#include "AIController.h"                     // Base AIController class
#include "AI_Main.h"                          // Forward access to the pawn class
#include "Perception/AIPerceptionTypes.h"     // FAIStimulus, enums, etc.
#include "Perception/AISenseConfig_Sight.h"   // Sight sense configuration

#include "AI_Controller.generated.h"          // UHT‑generated boilerplate

/**
 *  AAI_Controller – drives a single enemy pawn.
 *  Spawned automatically when the pawn is placed in the level (Auto Possess AI)
 *  or when you spawn the pawn at runtime.
 */
UCLASS()
class SPM_GROUPPROJECT_API AAI_Controller : public AAIController
{
    GENERATED_BODY()

public:
    /** Constructor – called once when the C++ class is instantiated by UClass. */
    explicit AAI_Controller(const FObjectInitializer& FObjectInitializer);

protected:
    // ~AController interface --------------------------------------------------
    /**
     *  Called the instant this controller takes possession of a pawn.
     *  Here we:
     *    1) Start the pawn's behaviour tree,
     *    2) Turn on crowd steering (optional),
     *    3) Cache a pointer to the blackboard for quick access.
     */
    virtual void OnPossess(APawn* InPawn) override;

private:
    /* ------- Perception ------- */
    /** Config object that defines our sight parameters (range, FOV, etc.) */
    UAISenseConfig_Sight* SightConfig = nullptr;

    /** Helper that builds the UAIPerceptionComponent and the SightConfig. */
    void SetupPerceptionSystem();

    /** Callback fired by UAIPerceptionComponent whenever a sensed actor changes
        state (becomes visible or not). Updates BB key accordingly. */
    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
};