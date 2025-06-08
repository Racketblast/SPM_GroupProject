// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FlyToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GridManager.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "FlyingAIHelper.h"
#include "FlyingEnemyAI.h"
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

    AFlyingEnemyAI* FlyingEnemy = Cast<AFlyingEnemyAI>(OwnerComp.GetAIOwner()->GetPawn());
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    if (FlyingEnemy && Player)
    {
        const FVector SmartTarget = FlyingAIHelper::GetSmartTargetLocation(Player, FlyingEnemy, FlyingEnemy->GetActorLocation()); 
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(TargetLocationKey.SelectedKeyName, SmartTarget);
    }
    
    FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TargetLocationKey.SelectedKeyName);
   // UE_LOG(LogTemp, Warning, TEXT("Flying enemy TargetLocation from BB: %s"), *TargetLocation.ToString());
    
    if (CurrentPath.Num() > 0 && FVector::Dist(TargetLocation, CurrentPath.Last()) < 100.f)
    {
        return EBTNodeResult::InProgress; //Fortsätte på den nuvarande pathen 
    }

    AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    if (!GridManager)
        return EBTNodeResult::Failed;

    CurrentPath = GridManager->FindPath(AIPawn->GetActorLocation(), TargetLocation);

    if (CurrentPath.Num() > 0)
    {
        bFallbackUsed = false; 
    }
    
    /*if (CurrentPath.Num() == 0 && !bFallbackUsed)
    {
        bFallbackUsed = true;
        // Fallback logic
        UE_LOG(LogTemp, Warning, TEXT("No path found to player, trying nearest walkable node."));

        TSharedPtr<FGridNode> FallbackNode = GridManager->FindNearestWalkableNode(AIPawn->GetActorLocation(), 2000.f);
    
        if (FallbackNode.IsValid())
        {
            CurrentPath = GridManager->FindPath(AIPawn->GetActorLocation(), FallbackNode->WorldLocation);
            if (CurrentPath.Num() == 0)
            {
                UE_LOG(LogTemp, Error, TEXT("Even fallback path failed."));
                FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            }

            UE_LOG(LogTemp, Warning, TEXT("Using fallback path to walkable node."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("No walkable fallback node found."));
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        }
    }*/

    // Ny debug
    //UE_LOG(LogTemp, Warning, TEXT("Path length: %d"), CurrentPath.Num());
    /*for (FVector Point : CurrentPath)
    {
        DrawDebugSphere(GetWorld(), Point, 25.0f, 12, FColor::Green, false, 2.0f);
    }*/
    
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
    if (CurrentPathIndex >= CurrentPath.Num())
    {
        bFallbackUsed = false;
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
    
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* AIPawn = AICon->GetPawn();
    if (!AIPawn) return;

    FVector CurrentPoint = CurrentPath[CurrentPathIndex];
    FVector Direction = (CurrentPoint - AIPawn->GetActorLocation()).GetSafeNormal();
    
    AIPawn->AddMovementInput(Direction, 1.0f, false);

    if (FVector::Dist(AIPawn->GetActorLocation(), CurrentPoint) < AcceptanceRadius)
    {
        CurrentPathIndex++;

        if (CurrentPathIndex >= CurrentPath.Num())
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}


