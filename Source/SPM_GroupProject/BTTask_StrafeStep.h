#pragma once  

#include "CoreMinimal.h"                                
#include "BehaviorTree/BTTaskNode.h"                   
#include "BTTask_StrafeStep.generated.h"               


UCLASS()
class SPM_GROUPPROJECT_API UBTTask_StrafeStep : public UBTTaskNode
{
    GENERATED_BODY()  

public:
    UBTTask_StrafeStep();  
    
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                            uint8*                  NodeMemory) override;
    
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

private:   
    FVector              TargetLocation{ForceInit};   
    class AAIController* CachedController = nullptr;  
    class ACharacter*    CachedPawn       = nullptr;  

    bool  bMoveRequested            = false;  
    bool  bSavedOrientRotToMovement = false;  
    float ElapsedTime               = 0.f;   
};