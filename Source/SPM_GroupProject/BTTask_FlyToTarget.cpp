// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FlyToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GridManager.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FlyToTarget::UBTTask_FlyToTarget()
{
    bNotifyTick = true;
    NodeName = "Fly To Target (Grid)";
}

EBTNodeResult::Type UBTTask_FlyToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* AIPawn = AICon ? AICon->GetPawn() : nullptr;
    if (!AIPawn)
        return EBTNodeResult::Failed;

    FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TargetLocationKey.SelectedKeyName);

    AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    if (!GridManager)
        return EBTNodeResult::Failed;

    CurrentPath = GridManager->FindPath(AIPawn->GetActorLocation(), TargetLocation);
    if (CurrentPath.Num() == 0)
        return EBTNodeResult::Failed;

    CurrentPathIndex = 0;
    return EBTNodeResult::InProgress;
}

void UBTTask_FlyToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* AIPawn = AICon ? AICon->GetPawn() : nullptr;
    if (!AIPawn || CurrentPath.Num() == 0)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    MoveToward(OwnerComp, DeltaSeconds);
}

void UBTTask_FlyToTarget::MoveToward(UBehaviorTreeComponent& OwnerComp, float DeltaSeconds)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* AIPawn = AICon->GetPawn();
    if (!AIPawn) return;

    FVector CurrentPoint = CurrentPath[CurrentPathIndex];
    FVector Direction = (CurrentPoint - AIPawn->GetActorLocation()).GetSafeNormal();
    float Speed = 600.f;

    //AIPawn->AddMovementInput(Direction, Speed * DeltaSeconds);
    AIPawn->AddMovementInput(Direction);

    if (FVector::Dist(AIPawn->GetActorLocation(), CurrentPoint) < AcceptanceRadius)
    {
        CurrentPathIndex++;

        if (CurrentPathIndex >= CurrentPath.Num())
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}


