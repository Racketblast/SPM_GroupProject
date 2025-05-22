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
											uint8* NodeMemory) override;
	
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
						  uint8*                 NodeMemory,
						  float                  DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Strafe")
	float StrafeDistance = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "Strafe")
	float StrafeVariance = 50.f;
	
	UPROPERTY(EditAnywhere, Category = "Strafe")
	float AcceptRadius = 40.f;

	/* timeout (s) */
	UPROPERTY(EditAnywhere, Category = "Strafe")
	float MaxMoveTime = 2.5f;

private:
	FVector              TargetLocation{ForceInit};
	class AAIController* CachedController = nullptr;
	class ACharacter*    CachedPawn       = nullptr;

	bool  bMoveRequested              = false;
	bool  bSavedOrientRotToMovement   = false;
	float ElapsedTime                 = 0.f;
};
