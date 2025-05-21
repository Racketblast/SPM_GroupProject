#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_JumpToNavMesh.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API UBTTask_JumpToNavMesh : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_JumpToNavMesh();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
