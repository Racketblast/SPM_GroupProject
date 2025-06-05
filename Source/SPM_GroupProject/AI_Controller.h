

#pragma once

#include "CoreMinimal.h"                       // Engine core types / utilities
#include "AIController.h"                     // Base AIController class
#include "AI_Main.h"                          // Forward access to the pawn class
#include "Perception/AIPerceptionTypes.h"     // FAIStimulus, enums, etc.
#include "Perception/AISenseConfig_Sight.h"   // Sight sense configuration

#include "AI_Controller.generated.h"          // UHT‑generated boilerplate


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