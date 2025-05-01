// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindPlayerLocation.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FindPlayerLocation::UBTTask_FindPlayerLocation(FObjectInitializer const& ObjectInitializer)
{
	NodeName = "Find Player Location";
}

EBTNodeResult::Type UBTTask_FindPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto* const Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		auto const PlayerLocation = Player->GetActorLocation();
		if (SearchRandom)
		{
			FNavLocation L;

			if (auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
			{
				if (NavSys->GetRandomPointInNavigableRadius(PlayerLocation, SearchRadius, L))
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), L.Location);
					FinishLatentTask(OwnerComp,EBTNodeResult::Succeeded);
					return EBTNodeResult::Succeeded;
				}
			}
			
		}
		else
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), PlayerLocation);
			FinishLatentTask(OwnerComp,EBTNodeResult::Succeeded);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
	//return Super::ExecuteTask(OwnerComp, NodeMemory);
}
