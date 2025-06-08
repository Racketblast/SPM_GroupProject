#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TeleportToNavmesh.generated.h"


UCLASS()
class SPM_GROUPPROJECT_API UBTTask_TeleportToNavmesh : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TeleportToNavmesh();
	
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8*                  NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Navigation")
	float SearchRadius = 500.0f;
};