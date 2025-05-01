// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindRandomLocation.h"

#include "AIController.h"
#include "AI_Controller.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

UBTTask_FindRandomLocation::UBTTask_FindRandomLocation(FObjectInitializer const& ObjectInitializer)
{
	NodeName = "Find Random Location In Navmesh";
}

EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	if (auto* const controller = Cast<AAI_Controller>(OwnerComp.GetAIOwner()))
	{
		if (auto* const ai = controller->GetPawn())
		{
			auto const Origin = ai->GetActorLocation();
			
			if (auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
			{
				FNavLocation L;
				if (NavSys->GetRandomPointInNavigableRadius(Origin, SearchRadius, L))
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), L.Location);
				}
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
	//return Super::ExecuteTask(OwnerComp, NodeMemory);
}
