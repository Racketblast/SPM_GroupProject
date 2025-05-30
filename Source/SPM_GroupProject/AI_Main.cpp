// Fill out your copyright notice in the Description page of Project Settings.

#include "AI_Main.h"

#include "MoneyBox.h"
#include "Components/AudioComponent.h"
#include "WaveManager.h"
#include "AI_Controller.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "EngineUtils.h"


/* ───────────────── CONSTRUCTOR ───────────────── */
AAI_Main::AAI_Main()
{
    PrimaryActorTick.bCanEverTick = true;             // Enable Tick()
    // Create & attach an AudioComponent so BP designers can drop SFX cues.
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AISoundComponent"));
}

/* ───────────────── GETTER ─────────────────────── */
UBehaviorTree* AAI_Main::GetBehaviorTree() const { return BehaviorTree; }

/* ───────────────── TAKE DAMAGE ────────────────── */
float AAI_Main::TakeDamage(float                      DamageAmount,
                           FDamageEvent const&        DamageEvent,
                           AController*               EventInstigator,
                           AActor*                    DamageCauser)
{
    // Early‑out if we’ve already processed death; prevents multiple Destroy() calls.
    if (bIsDead) { return 0.f; }

    AIHealth -= static_cast<int32>(DamageAmount);
    UE_LOG(LogTemp, Verbose, TEXT("%s took %.1f dmg → %d HP"),
           *GetName(), DamageAmount, AIHealth);

    /* ------------- Hit VFX ----------------------- */
    if (DamageEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), DamageEffect, GetActorLocation());
    }

    /* ------------- Death check ------------------- */
    if (AIHealth <= 0)
    {
        AIHealth = 0;
        bIsDead  = true;

        /* --------- Loot drop -------- */
        if (AIDrop)
        {
            // Spawn loot with zero rotation so boxes sit flat.
            FTransform SpawnTM = GetTransform();
            SpawnTM.SetRotation(FQuat::Identity);
            GetWorld()->SpawnActor<ACollectableBox>(AIDrop, SpawnTM);
        }

        /* --------- Wave manager notification ------ */
        for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
        {
            (*It)->OnEnemyKilled();
            break;      // Only the first WM is relevant.
        }

        /* --------- Death VFX -------- */
        if (DeathEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), DeathEffect, GetActorLocation());
        }

        Destroy();      // Remove pawn; controller will be automatically detached.
    }

    return DamageAmount;   // Base implementation expects we echo back the amount taken.
}

/* ───────────────── BEGIN PLAY ─────────────────── */
void AAI_Main::BeginPlay()
{
    Super::BeginPlay();

    AIHealth = MaxAIHealth;            // Reset health after construction defaults

    /*  --------- Crowd avoidance tuning ---------- */
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        // Enable Reciprocal Velocity Obstacles (RVO) so groups flow around each other.
        Move->bUseRVOAvoidance             = true;
        Move->AvoidanceConsiderationRadius = 500.f;  // How far ahead pawn “looks”
        Move->AvoidanceWeight              = 0.7f;   // Lower → more easily yields
    }

    LastKnownLocation = GetActorLocation();   // Seed stuck check
}

/* ───────────────── TICK ───────────────────────── */
void AAI_Main::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    /*  --------- Ignore while airborne ---------- */
    if (UCharacterMovementComponent* Move = GetCharacterMovement();
        Move && (Move->IsFalling() || Move->IsFlying()))
    {
        return;
    }

    /*  --------- Reset stuck timer vid firing -- */
    if (AAI_Controller* AICont = Cast<AAI_Controller>(GetController()))
    {
        if (UBlackboardComponent* BB = AICont->GetBlackboardComponent())
        {
            if (BB->GetValueAsBool(FName("IsFiring")))
            {
                // reset timers vid IsFiring
                TimeSinceLastMovement = 0.f;
                LastKnownLocation     = GetActorLocation();
                return;
            }
        }
    }

    /*  --------- Detect lack of movement --------- */
    const FVector Curr = GetActorLocation();
    if (FVector::DistSquared(Curr, LastKnownLocation) > FMath::Square(MinMoveDistance))
    {
        // Pawn moved enough → reset timers.
        LastKnownLocation     = Curr;
        TimeSinceLastMovement = 0.f;
    }
    else
    {
        TimeSinceLastMovement += DeltaTime;
    }

    /*  --------- Krav för JumpToNavmesh ------ */
    if (TimeSinceLastMovement > StuckCheckInterval || IsOutsideNavMesh())
    {
        if (AAI_Controller* AICont = Cast<AAI_Controller>(GetController()))
        {
            //Nullptr
            if (UBlackboardComponent* BB = AICont->GetBlackboardComponent())
            {
                BB->SetValueAsBool(FName("TeleportToNavmesh"), true);
            }
        }
    }
}

/* ───────────────── INPUT BINDINGS ─────────────── */
void AAI_Main::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // No direct input for AI pawns, but keeping override for future debugging.
}

/* ───────────────── NAVMESH TEST ─────────────────  */
/* Kallas från tick ovan
  En av kraven för JumpToNavmesh */
bool AAI_Main::IsOutsideNavMesh() const
{
    const UNavigationSystemV1* Nav =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!Nav) { return false; }

    FNavLocation Dummy;
    return !Nav->ProjectPointToNavigation(GetActorLocation(), Dummy);
}
