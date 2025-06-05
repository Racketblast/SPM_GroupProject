#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TeleportToNavmesh.generated.h"


UCLASS()
class SPM_GROUPPROJECT_API UBTTask_TeleportToNavmesh : public UBTTaskNode
{
	GENERATED_BODY()

public:
	// Constructor only assigns a human‑readable node name so designers see a
	// friendly label in the BT editor.
	UBTTask_TeleportToNavmesh();

	// Core execution entry – instantly succeeds or fails; no ticking.
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8*                  NodeMemory) override;

protected:
	// How far from the pawn we should look for a valid spot on the NavMesh.
	// Exposed so designers can tweak in the editor per BT instance.
	UPROPERTY(EditAnywhere, Category = "Navigation")
	float SearchRadius = 500.0f;
};