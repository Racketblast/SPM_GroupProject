

#pragma once

#include "CoreMinimal.h"                          // UE core types / macros
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h" // parent class
#include "BehaviorTree/BlackboardComponent.h"         // Blackboard helpers
#include "BTTask_FindRandomLocation.generated.h"       // UHT‑generated include


UCLASS()
class SPM_GROUPPROJECT_API UBTTask_FindRandomLocation : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    explicit UBTTask_FindRandomLocation(const FObjectInitializer& ObjectInitializer);
    
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                            uint8* NodeMemory) override;

private:
    UPROPERTY(EditAnywhere, Category = "AI", meta = (AllowPrivateAccess = "true"))
    float SearchRadius = 1500.f;
};