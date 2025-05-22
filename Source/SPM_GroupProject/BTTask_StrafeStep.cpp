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
	CachedController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!CachedController) return EBTNodeResult::Failed;

	CachedPawn = Cast<ACharacter>(CachedController->GetPawn());
	if (!CachedPawn) return EBTNodeResult::Failed;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(CachedPawn, 0);
	if (!Player) return EBTNodeResult::Failed;
	
	const FVector PawnLoc  = CachedPawn->GetActorLocation();
	const FVector ToPlayer = (Player->GetActorLocation() - PawnLoc).GetSafeNormal2D();
	const FVector RightDir = FVector::CrossProduct(FVector::UpVector, ToPlayer);

	const float SideSign = (FMath::RandBool() ? 1.f : -1.f);
	const float Dist     = StrafeDistance + FMath::FRandRange(-StrafeVariance, StrafeVariance);

	FVector Candidate = PawnLoc + RightDir * SideSign * Dist;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation          NavLoc;

	auto IsValid = [&](const FVector& Test)->bool
	{
		if (!NavSys || !NavSys->ProjectPointToNavigation(Test, NavLoc, FVector(100,100,200)))
			return false;

		FCollisionQueryParams P(SCENE_QUERY_STAT(StrafeLOS), true, CachedPawn);
		FHitResult Hit;
		bool Blocked = GetWorld()->LineTraceSingleByChannel(
		    Hit, NavLoc.Location, Player->GetActorLocation(),
		    ECC_Visibility, P);
		return !Blocked;
	};

	if (!IsValid(Candidate))
	{
		Candidate = PawnLoc - RightDir * SideSign * Dist;
		if (!IsValid(Candidate))
			return EBTNodeResult::Failed;
	}
	TargetLocation = NavLoc.Location;

	/* turn towards player & disable movement rotation */
	CachedController->SetFocus(Player, EAIFocusPriority::Gameplay);

	if (UCharacterMovementComponent* Move = CachedPawn->GetCharacterMovement())
	{
		bSavedOrientRotToMovement     = Move->bOrientRotationToMovement;
		Move->bOrientRotationToMovement = false;
	}

	/* MoveTo */
	FAIMoveRequest Req(TargetLocation);
	Req.SetAcceptanceRadius(AcceptRadius);
	Req.SetUsePathfinding(true);

	FNavPathSharedPtr PathPtr;
	FPathFollowingRequestResult R = CachedController->MoveTo(Req, &PathPtr);
	if (R.Code == EPathFollowingRequestResult::Type::Failed)
	{
		CachedController->ClearFocus(EAIFocusPriority::Gameplay);
		if (auto* Move = CachedPawn->GetCharacterMovement())
			Move->bOrientRotationToMovement = bSavedOrientRotToMovement;
		return EBTNodeResult::Failed;
	}

	bMoveRequested = true;
	ElapsedTime    = 0.f;
	return EBTNodeResult::InProgress;
}

/* ───────────────────────────────────────────── */
void UBTTask_StrafeStep::TickTask(UBehaviorTreeComponent& OwnerComp,
                                  uint8*                 /*NodeMemory*/,
                                  float                  DeltaSeconds)  
{
	if (!bMoveRequested || !CachedPawn) return;

	ElapsedTime += DeltaSeconds;
	const float Dist = FVector::DistXY(CachedPawn->GetActorLocation(), TargetLocation);

	auto Finish = [&](EBTNodeResult::Type Result)
	{
		CachedController->StopMovement();
		CachedController->ClearFocus(EAIFocusPriority::Gameplay);

		if (auto* Move = CachedPawn->GetCharacterMovement())
			Move->bOrientRotationToMovement = bSavedOrientRotToMovement;

		FinishLatentTask(OwnerComp, Result);
	};

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
