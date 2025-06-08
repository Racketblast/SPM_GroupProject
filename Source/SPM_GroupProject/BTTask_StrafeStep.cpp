#include "BTTask_StrafeStep.h"  
#include "AIController.h"                                  
#include "BehaviorTree/BlackboardComponent.h"              
#include "Kismet/GameplayStatics.h"                        
#include "NavigationSystem.h"                              
#include "Navigation/PathFollowingComponent.h"            
#include "GameFramework/Character.h"                       
#include "GameFramework/CharacterMovementComponent.h"      

/* ───────────────────────────────────────────── */
UBTTask_StrafeStep::UBTTask_StrafeStep()
{
    NodeName    = TEXT("Strafe Step (Face Player)");  
    bNotifyTick = true;                               
}

/* ───────────────────────────────────────────── */
EBTNodeResult::Type UBTTask_StrafeStep::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* )
{
    // Cache controller + pawn for fast access throughout the task ------------
    CachedController = Cast<AAIController>(OwnerComp.GetAIOwner());
    if (!CachedController) return EBTNodeResult::Failed;

    CachedPawn = Cast<ACharacter>(CachedController->GetPawn());
    if (!CachedPawn) return EBTNodeResult::Failed;

    // Acquire player reference ------------------------------------------------
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(CachedPawn, 0);
    if (!Player) return EBTNodeResult::Failed;
    
    // Build a right‑vector perpendicular to the player direction (XY plane)
    const FVector PawnLoc  = CachedPawn->GetActorLocation();
    const FVector ToPlayer = (Player->GetActorLocation() - PawnLoc).GetSafeNormal2D();
    FVector RightDir = FVector::CrossProduct(FVector::UpVector, ToPlayer);
    RightDir.Z = 0.f;
    RightDir.Normalize();

    // Pick L or R randomly & add distance variance ---------------------------
    const float SideSign = (FMath::RandBool() ? 1.f : -1.f);
    const float Dist     = StrafeDistance + FMath::FRandRange(-StrafeVariance, StrafeVariance);

    FVector Candidate = PawnLoc + RightDir * SideSign * Dist;  // first try

    // Utility lambdas ---------------------------------------------------------
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    FNavLocation          NavLoc;

    auto IsValid = [&](const FVector& Test)->bool
    {
        // 1) Must project onto NavMesh ---------------------------------------
        if (!NavSys || !NavSys->ProjectPointToNavigation(Test, NavLoc, FVector(100,100,200)))
            return false;

        // 2) Must have clear LOS to player -----------------------------------
        FCollisionQueryParams P(SCENE_QUERY_STAT(StrafeLOS), true, CachedPawn);
        FHitResult Hit;
        bool Blocked = GetWorld()->LineTraceSingleByChannel(
            Hit, NavLoc.Location, Player->GetActorLocation(),
            ECC_Visibility, P);
        return !Blocked;
    };

    // Validate first candidate; if blocked, try the opposite side ------------
    if (!IsValid(Candidate))
    {
        Candidate = PawnLoc - RightDir * SideSign * Dist;
        if (!IsValid(Candidate))
            return EBTNodeResult::Failed;  // give up
    }
    TargetLocation = NavLoc.Location;

    /* turn towards player & disable movement rotation */
    CachedController->SetFocus(Player, EAIFocusPriority::Gameplay);

    if (UCharacterMovementComponent* Move = CachedPawn->GetCharacterMovement())
    {
        bSavedOrientRotToMovement   = Move->bOrientRotationToMovement;
        Move->bOrientRotationToMovement = false;  // keep facing player
    }

    /* Issue MoveTo -----------------------------------------------------------*/
    FAIMoveRequest Req(TargetLocation);
    Req.SetAcceptanceRadius(AcceptRadius);
    Req.SetUsePathfinding(true);

    FNavPathSharedPtr PathPtr;  // not used but required by API
    FPathFollowingRequestResult R = CachedController->MoveTo(Req, &PathPtr);
    if (R.Code == EPathFollowingRequestResult::Type::Failed)
    {
        // Clean up orientation flags/focus before exiting --------------------
        CachedController->ClearFocus(EAIFocusPriority::Gameplay);
        if (auto* Move = CachedPawn->GetCharacterMovement())
            Move->bOrientRotationToMovement = bSavedOrientRotToMovement;
        return EBTNodeResult::Failed;
    }

    // Success so far – start ticking -----------------------------------------
    bMoveRequested = true;
    ElapsedTime    = 0.f;
    return EBTNodeResult::InProgress;
}

/* ───────────────────────────────────────────── */
void UBTTask_StrafeStep::TickTask(UBehaviorTreeComponent& OwnerComp,
                                  uint8*                 /*NodeMemory*/,
                                  float                  DeltaSeconds)  
{
    if (!bMoveRequested || !CachedPawn) return;  // nothing to do

    ElapsedTime += DeltaSeconds;  // progress timer for timeout
    const float Dist = FVector::DistXY(CachedPawn->GetActorLocation(), TargetLocation);

    // Local lambda to end task + restore state -------------------------------
    auto Finish = [&](EBTNodeResult::Type Result)
    {
        CachedController->StopMovement();
        CachedController->ClearFocus(EAIFocusPriority::Gameplay);

        if (auto* Move = CachedPawn->GetCharacterMovement())
            Move->bOrientRotationToMovement = bSavedOrientRotToMovement;

        FinishLatentTask(OwnerComp, Result);
    };

    // Check finish conditions -------------------------------------------------
    if (Dist <= AcceptRadius)
    {
        Finish(EBTNodeResult::Succeeded);
        return;
    }

    if (ElapsedTime > MaxMoveTime ||
        CachedController->GetMoveStatus() == EPathFollowingStatus::Type::Idle)
    {
        Finish(EBTNodeResult::Failed);
    }
}
