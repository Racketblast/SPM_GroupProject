#include "BTTask_StopMovement.h"
#include "AI_Controller.h"
#include "AI_Main.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UBTTask_StopMovement::UBTTask_StopMovement()
{
	NodeName = TEXT("Stop Movement");
}

EBTNodeResult::Type UBTTask_StopMovement::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* Cont = Cast<AAIController>(OwnerComp.GetAIOwner()))
	{
		//  stoppar chase movement utan att påverka physics och navlink moves
		if (Cont->GetPathFollowingComponent())
		{
			//Avbryter dens aktiva "path request"
			Cont->StopMovement();
			//Move to playerlocation från chase blir cancelled om man clearar playerlocation
			OwnerComp.GetBlackboardComponent()->ClearValue(FName("PlayerLocation"));

		}

		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
