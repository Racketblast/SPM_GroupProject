#include "BTTask_JumpOnLink.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

UBTTask_JumpOnLink::UBTTask_JumpOnLink()
{
    NodeName = "Jump On Smart Link";
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_JumpOnLink::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    ACharacter* Character = Cast<ACharacter>(Controller ? Controller->GetPawn() : nullptr);

    if (!Controller || !Character)
    {
        return EBTNodeResult::Failed;
    }

    UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
    if (!PathComp || !PathComp->HasValidPath())
    {
        return EBTNodeResult::Failed;
    }

    FNavPathSharedPtr Path = PathComp->GetPath();
    if (!Path.IsValid()) return EBTNodeResult::Failed;

    const TArray<FNavPathPoint>& Points = Path->GetPathPoints();
    const int32 CurrentIndex = PathComp->GetCurrentPathIndex();

    if (Points.IsValidIndex(CurrentIndex))
    {
        const FNavPathPoint& Point = Points[CurrentIndex];

        // ✅ Use IsValid() to check smart link presence
        if (Point.CustomNavLinkId.IsValid())
        {
            FVector Direction = Character->GetActorForwardVector();
            FVector LaunchVelocity = Direction * 300.f + FVector(0, 0, 600.f);
            Character->LaunchCharacter(LaunchVelocity, true, true);

            bJumped = true;
            ElapsedTime = 0.f;
            return EBTNodeResult::InProgress;
        }
    }


    return EBTNodeResult::Succeeded;
}

void UBTTask_JumpOnLink::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (!bJumped)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    ElapsedTime += DeltaSeconds;
    if (ElapsedTime > 0.5f)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
