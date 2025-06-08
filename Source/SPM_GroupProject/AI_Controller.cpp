
#include "AI_Controller.h"
#include "PlayerCharacter.h"                    // For Cast< APlayerCharacter >
#include "BehaviorTree/BlackboardComponent.h"   // Blackboard access from code
#include "Perception/AIPerceptionComponent.h"   // UAIPerceptionComponent class
#include "Navigation/CrowdFollowingComponent.h" // Crowd steering behaviour
#include "Navigation/PathFollowingComponent.h"  // Base path following

// ───────────────────────────────────────────────────────────── Constructor ─── //
AAI_Controller::AAI_Controller(const FObjectInitializer& FObjectInitializer)
    : Super(FObjectInitializer)            
{
    // bygger perception tidigt så den finns innan possession.
    SetupPerceptionSystem();
}

// ───────────────────────────────────────────────────────────── OnPossess ──── //
void AAI_Controller::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    SetMoveBlockDetection(true);
    
    if (AAI_Main* AI = Cast<AAI_Main>(InPawn))
    {
        
        /* -------------------------------------------------- Behaviour Tree */
        if (UBehaviorTree*  BT = AI->GetBehaviorTree())
        {
            UBlackboardComponent* NewBB = nullptr;
            UseBlackboard(BT->BlackboardAsset, NewBB);
            Blackboard = NewBB;               
            RunBehaviorTree(BT);              
        }
        
        if (UCrowdFollowingComponent* Crowd =
                Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
        {
            Crowd->SuspendCrowdSteering(false);   // false = aktiv
        }
    }
}

// ───────────────────────────────────────────────────── SetupPerceptionSystem ─── //
//bygger UAIPerceptionComponent + en sight sense configuration, kopplar sen ihop dom så vi får OnTargetPerceptionUpdated events.
void AAI_Controller::SetupPerceptionSystem()
{
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    
    UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*Perception);

    /* -------------------- vision parameters ------------------------ */
    SightConfig->SightRadius                 = 15000.0f;  
    SightConfig->PeripheralVisionAngleDegrees = 360.f;     
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;

    /* ------------------ Hook up the perception component --------------- */
    Perception->ConfigureSense(*SightConfig);
    Perception->SetDominantSense(*SightConfig->GetSenseImplementation());

    // Bind C++ callback for when perception state changes.
    Perception->OnTargetPerceptionUpdated.AddDynamic(this, &AAI_Controller::OnTargetDetected);
}

// ───────────────────────────────────────────────────── OnTargetDetected ─── //
// Kallad vid sensed or lost.

void AAI_Controller::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (const APlayerCharacter* Player = Cast<APlayerCharacter>(Actor))
    {
        Blackboard->SetValueAsBool("CanSeePlayerCharacter", Stimulus.WasSuccessfullySensed()); // true on sight-gained, false on lost
    }
}
