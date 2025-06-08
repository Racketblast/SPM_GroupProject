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
    explicit AAI_Controller(const FObjectInitializer& FObjectInitializer);

protected:
    virtual void OnPossess(APawn* InPawn) override;

private:
    UAISenseConfig_Sight* SightConfig = nullptr;
    
    void SetupPerceptionSystem();
    
    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
};