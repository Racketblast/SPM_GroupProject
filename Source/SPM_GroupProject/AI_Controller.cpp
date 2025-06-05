
#include "AI_Controller.h"
#include "PlayerCharacter.h"                    // For Cast< APlayerCharacter >
#include "BehaviorTree/BlackboardComponent.h"   // Blackboard access from code
#include "Perception/AIPerceptionComponent.h"   // UAIPerceptionComponent class
#include "Navigation/CrowdFollowingComponent.h" // Crowd steering behaviour
#include "Navigation/PathFollowingComponent.h"  // Base path following

// ───────────────────────────────────────────────────────────── Constructor ─── //
AAI_Controller::AAI_Controller(const FObjectInitializer& FObjectInitializer)
    : Super(FObjectInitializer)            // base‑class init list
{
    // Build perception as early as possible so it exists before possession.
    SetupPerceptionSystem();
}

// ───────────────────────────────────────────────────────────── OnPossess ──── //
void AAI_Controller::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 1) Make sure we actually possess our expected pawn class.
    if (AAI_Main* AI = Cast<AAI_Main>(InPawn))
    {
        
        /* -------------------------------------------------- Behaviour Tree */
        if (UBehaviorTree*  BT = AI->GetBehaviorTree())
        {
            // UseBlackboard returns true/false *and* gives you the component.
            UBlackboardComponent* NewBB = nullptr;
            UseBlackboard(BT->BlackboardAsset, NewBB);
            Blackboard = NewBB;               // cache for quick access
            RunBehaviorTree(BT);              // fire it up!
        }

        /* ------------------------------------------------------ Navigation */
        // Enable CrowdFollowing if the PathFollowingComponent supports it.
        // (Crowd steering lets multiple AI avoid each other smoothly.)
        if (UCrowdFollowingComponent* Crowd =
                Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
        {
            Crowd->SuspendCrowdSteering(false);   // false = aktiv
        }
    }
}

// ───────────────────────────────────────────────────── SetupPerceptionSystem ─── //
/**
 * Builds a UAIPerceptionComponent + a sight sense configuration, then wires
 * them together so we receive OnTargetPerceptionUpdated events.
 */
void AAI_Controller::SetupPerceptionSystem()
{
    // 1) Create the sense config object. Needs a *stable* pointer for UObjects, (engine deletes it with the controller).
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    // 2) Create the perception component itself and declare that sight is, its dominant (primary) sense.
    UAIPerceptionComponent* Perception =
        CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*Perception);

    /* -------------------- Tune vision parameters ------------------------ */
    SightConfig->SightRadius                 = 15000.0f;  // max range
    SightConfig->PeripheralVisionAngleDegrees = 90.f;     // 180° total FOV
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;

    /* ------------------ Hook up the perception component --------------- */
    Perception->ConfigureSense(*SightConfig);
    Perception->SetDominantSense(*SightConfig->GetSenseImplementation());

    // Bind C++ callback for when perception state changes.
    Perception->OnTargetPerceptionUpdated.AddDynamic(
        this, &AAI_Controller::OnTargetDetected);
}

// ───────────────────────────────────────────────────── OnTargetDetected ─── //
// Called whenever the perception component registers that an actor was either successfully sensed or lost.

void AAI_Controller::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (const APlayerCharacter* Player = Cast<APlayerCharacter>(Actor))
    {
        // Blackboard key name is hard‑coded here; consider making it a const.
        Blackboard->SetValueAsBool("CanSeePlayerCharacter", Stimulus.WasSuccessfullySensed()); // true on sight-gained, false on lost
    }
}
