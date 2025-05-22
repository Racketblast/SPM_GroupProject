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
#include "Components/CapsuleComponent.h"

/* ─────────────────────────────────────────────── */
/*                   CONSTRUCTOR                   */
/* ─────────────────────────────────────────────── */
AAI_Main::AAI_Main()
{
	PrimaryActorTick.bCanEverTick = true;
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AISoundComponent"));
}

/* ─────────────────────────────────────────────── */
UBehaviorTree* AAI_Main::GetBehaviorTree() const { return BehaviorTree; }

/* ─────────────────────────────────────────────── */
/*                    DAMAGE                       */
/* ─────────────────────────────────────────────── */
float AAI_Main::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                           AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsDead)
    {
        AIHealth -= DamageAmount;
        UE_LOG(LogTemp, Warning, TEXT("AI TakeDamage %.1f"), DamageAmount);

        if (DamageEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DamageEffect, GetActorLocation());
        }

        if (AIHealth <= 0)
        {
            AIHealth = 0;
            bIsDead = true;

            // Notify wave manager
            for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
            {
                (*It)->OnEnemyKilled();
                break;
            }

            // Play Niagara death effect
            if (DeathEffect)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DeathEffect, GetActorLocation());
            }

            // Drop item
        	if (AIDrop)
        	{
        		FTransform T = GetTransform();
        		T.SetRotation({0, 0, 0, 0});
        		GetWorld()->SpawnActor<ACollectableBox>(AIDrop, T);
        	}

            // Disable character movement
            GetCharacterMovement()->DisableMovement();
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            // Stop AI logic
            if (AAIController* AICont = Cast<AAIController>(GetController()))
            {
                AICont->StopMovement();
                AICont->UnPossess();
            }

            // Enable ragdoll physics
        	if (USkeletalMeshComponent* MeshComp = GetMesh())
        	{
		        if (MeshComp->GetSkeletalMeshAsset())
		        {
		        	UE_LOG(LogTemp, Warning, TEXT("AI Killed"));
		        	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		        	MeshComp->SetSimulatePhysics(true);
		        	MeshComp->SetAllBodiesSimulatePhysics(true);
		        	MeshComp->WakeAllRigidBodies();
		        	MeshComp->bBlendPhysics = true;

		        	// Calculate impulse direction (from the hit location)
		        	FVector ImpulseDirection = GetActorLocation() - DamageCauser->GetActorLocation();
		        	ImpulseDirection.Normalize();

		        	float ImpulseStrength = 20000.f; // Adjust based on how dramatic you want the effect
		        	FVector Impulse = ImpulseDirection * ImpulseStrength;
		        	AIDamage = 0;

		        	FVector HitLocation = GetActorLocation(); // You can improve this if you have a real hit point

		        	MeshComp->AddImpulseAtLocation(Impulse, HitLocation);

		        	// Optional: Destroy after delay
		        	SetLifeSpan(10.f);
		        }
		        else
		        {
			        Destroy();
		        }
        	}

            // Optional: Destroy after some delay
            SetLifeSpan(10.0f); // Character will be auto-destroyed after 10 seconds
        }
    }

    return DamageAmount;
}

/* ─────────────────────────────────────────────── */
void AAI_Main::BeginPlay()
{
	Super::BeginPlay();
	AIHealth = MaxAIHealth;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bUseRVOAvoidance             = true;
		Move->AvoidanceConsiderationRadius = 500.f;
		Move->AvoidanceWeight              = 0.7f;
	}	//AI Crowd avoidance attempt

	LastKnownLocation = GetActorLocation();
}

/* ─────────────────────────────────────────────── */
void AAI_Main::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*  Skip when jumping or flying */
	if (UCharacterMovementComponent* Move = GetCharacterMovement();
	    Move && (Move->IsFalling() || Move->IsFlying()))
	{
		return;
	}

	/*  Skip “stuck” while AI is firing */
	if (AAI_Controller* AIC = Cast<AAI_Controller>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			if (BB->GetValueAsBool(FName("IsFiring")))
			{
				TimeSinceLastMovement = 0.f;
				LastKnownLocation     = GetActorLocation();
				return;                                   
			}
		}
	}

	/* stuck-detection */
	const FVector Curr = GetActorLocation();
	if (FVector::DistSquared(Curr, LastKnownLocation) >
	    MinMoveDistance * MinMoveDistance)
	{
		LastKnownLocation     = Curr;
		TimeSinceLastMovement = 0.f;
	}
	else
	{
		TimeSinceLastMovement += DeltaTime;
	}

	if (TimeSinceLastMovement > StuckCheckInterval || IsOutsideNavMesh())
	{
		if (AAI_Controller* AICont = Cast<AAI_Controller>(GetController()))
		{
			if (UBlackboardComponent* BB = AICont->GetBlackboardComponent())
			{
				BB->SetValueAsBool(FName("TeleportToNavmesh"), true);
			}
		}
	}
}

/* ─────────────────────────────────────────────── */
void AAI_Main::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

/* ─────────────────────────────────────────────── */
bool AAI_Main::IsOutsideNavMesh() const
{
	const UNavigationSystemV1* Nav =
	    FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Nav) return false;

	FNavLocation Dummy;
	return !Nav->ProjectPointToNavigation(GetActorLocation(), Dummy);
}
