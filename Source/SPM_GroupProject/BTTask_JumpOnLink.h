#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_JumpOnLink.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API UBTTask_JumpOnLink : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_JumpOnLink();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	bool bJumped = false;
	float ElapsedTime = 0.f;
};
