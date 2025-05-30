// ───────────────────────────────────────────────────────────────────────────── //
//  BTTask_FireEnemyProjectile
//  ---------------------------------------------------------------------------
//  Behavior‑tree task that makes an AI pawn:
//    1. Face the player character,
//    2. Spawn a projectile toward that player, and
//    3. Toggle a Blackboard boolean ("IsFiring") for the duration of a cooldown
//       so that other BT decorators/services know the pawn is busy firing.
//
//  Key implementation details
//  --------------------------
//  • A single FTimerHandle (ResetFireHandle) prevents multiple overlapping
//    timers that would otherwise look like a memory leak if this task were
//    executed rapidly.
//  • We capture the Blackboard component via TWeakObjectPtr inside the timer
//    delegate so the delegate does *not* stop the component from being garbage
//    collected if the AI controller is destroyed.
//  • All tunables (cool‑down, muzzle offsets, projectile class) are exposed as
//    UPROPERTY so designers can tweak them in the editor.
//
//  Author : Gabriel
//  Date   : 2025‑05‑29
// ───────────────────────────────────────────────────────────────────────────── //

#include "BTTask_FireEnemyProjectile.h"

// Engine / Gameplay
#include "AI_Controller.h"                    // Custom AIController owning this task
#include "AI_Main.h"                          // Pawn class used by the project
#include "Projectile.h"                       // Projectile to spawn
#include "PlayerCharacter.h"                  // Player pawn we aim at
#include "BehaviorTree/BlackboardComponent.h" // Blackboard access helpers
#include "Kismet/GameplayStatics.h"           // GetPlayerCharacter helper
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"                     // FTimerHandle utilities

// ───────────────────────────────────────────────────────────────────────────── //
//  Constructor – set a friendly node name so level designers can read it in BT
// ───────────────────────────────────────────────────────────────────────────── //
UBTTask_FireEnemyProjectile::UBTTask_FireEnemyProjectile()
{
    NodeName = TEXT("Fire Enemy Projectile");
}

// ───────────────────────────────────────────────────────────────────────────── //
//  ExecuteTask
//  -----------
//  * Validate we have the pieces we need (controller, pawn, projectile class).
//  * Raise the IsFiring flag on the blackboard.
//  * Arm a *single* non‑looping timer that will reset the flag after the
//    configured cool‑down.  If the timer is already active we skip re‑arming.
//  * Rotate the pawn toward the player and spawn the projectile.
// ───────────────────────────────────────────────────────────────────────────── //
EBTNodeResult::Type UBTTask_FireEnemyProjectile::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                            uint8* /*NodeMemory*/)
{
    // ── Basic sanity checks ───────────────────────────────────────────────── //
    AAI_Controller* Controller = Cast<AAI_Controller>(OwnerComp.GetAIOwner());
    if (!Controller) { return EBTNodeResult::Failed; }

    // Pointer to the pawn we control (cast to game‑specific class)
    AAI_Main* AI = Cast<AAI_Main>(Controller->GetPawn());
    if (!AI || !ProjectileClass) { return EBTNodeResult::Failed; }

    // Blackboard component – needed to set the firing flag
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) { return EBTNodeResult::Failed; }

    // ── 1. Raise the IsFiring flag  ───────────────────────────────────────── //
    // Using TEXT() macro so we avoid a temporary FName for every call.
    BB->SetValueAsBool(TEXT("IsFiring"), true);

    // ── 2. Schedule flag reset if no timer is already pending ─────────────── //
    if (!AI->GetWorldTimerManager().IsTimerActive(ResetFireHandle))
    {
        // Capture the BB weakly so the delegate won't extend its lifetime.
        TWeakObjectPtr<UBlackboardComponent> WeakBB(BB);

        // Timer delegate executed after FiringCooldown seconds
        FTimerDelegate ResetDel = FTimerDelegate::CreateWeakLambda(
            AI /*owner tracked weakly by the delegate*/, [WeakBB]()
            {
                if (WeakBB.IsValid())
                {
                    WeakBB->SetValueAsBool(TEXT("IsFiring"), false);
                }
            });

        AI->GetWorldTimerManager().SetTimer(/*out*/ResetFireHandle,
                                            ResetDel,
                                            FiringCooldown,
                                            /*bLoop=*/false);
    }

    // ── 3. Face the player before firing ──────────────────────────────────── //
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(AI, 0);
    if (!Player) { return EBTNodeResult::Failed; }

    const FVector AILoc     = AI->GetActorLocation();
    const FVector PlayerLoc = Player->GetActorLocation();

    // Only yaw – no pitch/roll so the pawn stays upright in 2D plane
    FRotator YawOnly = (PlayerLoc - AILoc).Rotation();
    YawOnly.Pitch = 0.f;
    YawOnly.Roll  = 0.f;

    AI->SetActorRotation(YawOnly);
    Controller->SetControlRotation(YawOnly);

    // ── 4. Calculate muzzle location and direction ───────────────────────── //
    const FVector MuzzleLoc =
        AILoc + AI->GetActorForwardVector() * MuzzleForwardOffset +
        FVector(0.f, 0.f, MuzzleUpOffset);

    const FVector Dir      = (PlayerLoc - MuzzleLoc).GetSafeNormal();
    const FRotator ShotRot = Dir.Rotation();

    // ── 5. Spawn the projectile ───────────────────────────────────────────── //
    FActorSpawnParameters Params;
    Params.Owner      = AI;   // Damage credit + network relevancy
    Params.Instigator = AI;   // For pawn‑based damage rules

    AProjectile* Proj = AI->GetWorld()->SpawnActor<AProjectile>(ProjectileClass,
                                                                MuzzleLoc,
                                                                ShotRot,
                                                                Params);

    if (!Proj)
    {
        return EBTNodeResult::Failed; // Could not spawn – out of pool, etc.
    }

    // Pass damage and velocity to the projectile instance
    Proj->ProjectileDamage = AI->AIDamage;
    if (UProjectileMovementComponent* Move = Proj->ProjectileComponent)
    {
        Move->Velocity = Dir * Move->InitialSpeed;
    }

    return EBTNodeResult::Succeeded;
}

