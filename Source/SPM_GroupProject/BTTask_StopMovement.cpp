#include "BTTask_StopMovement.h"
#include "AI_Controller.h"
#include "AI_Main.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UBTTask_StopMovement::UBTTask_StopMovement()
{
	NodeName = TEXT("Stop Movement");
}

EBTNodeResult::Type UBTTask_StopMovement::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAI_Controller* Controller = Cast<AAI_Controller>(OwnerComp.GetAIOwner()))
	{
		if (AAI_Main* AICharacter = Cast<AAI_Main>(Controller->GetPawn()))
		{
			// Stop the character's movement
			AICharacter->GetCharacterMovement()->StopMovementImmediately();
			
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
