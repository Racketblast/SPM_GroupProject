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
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	ACharacter* CachedCharacter;

	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;

	float ElapsedTime = 0.0f;
	const float MaxWaitTime = 3.0f;
};
