#include "AI_Main.h"

#include "MoneyBox.h"
#include "Components/AudioComponent.h"
#include "WaveManager.h"
#include "AI_Controller.h"
#include "ChallengeSubsystem.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "EngineUtils.h"
#include "Explosive.h"
#include "FlyingEnemyAI.h"
#include "MeleeDamageType.h"
#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"

/* ─────────────────────────────────────────────── */
/*                   CONSTRUCTOR                   */
/* ─────────────────────────────────────────────── */
AAI_Main::AAI_Main()
{
	PrimaryActorTick.bCanEverTick = true;
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AISoundComponent"));
	AudioComponent->SetupAttachment(RootComponent); 
}

/* ─────────────────────────────────────────────── */
UBehaviorTree* AAI_Main::GetBehaviorTree() const { return BehaviorTree; }

/* ─────────────────────────────────────────────── */
/*                    DAMAGE                       */
/* ─────────────────────────────────────────────── */
float AAI_Main::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                           AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
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
        	OnEnemyDied.Broadcast(this);
        	
        	// Notify challenge, Kollar om det var en granat som dödade fienden
        	if (DamageCauser && DamageCauser->IsA<AExplosive>())
        	{
        		if (APlayerCharacter* Player = Cast<APlayerCharacter>(EventInstigator->GetPawn()))
        		{
        			if (UChallengeSubsystem* ChallengeSystem = Player->GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
        			{
        				ChallengeSystem->OnEnemyKilledWithGrenade();
        			}
        		}
        	}
        	
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
        	if (AIDrop[0] && AIDrop.Num() > 0)
        	{
        		FTransform T = GetTransform();
        		T.SetRotation({0, 0, 0, 0});
        		if (DamageEvent.DamageTypeClass == UMeleeDamageType::StaticClass())
        		{
        			UE_LOG(LogTemp, Display, TEXT("Melee kill"));
			        if (AIDrop[1])
			        {
			        	if (AIDrop.Num() > 0)
			        	{
			        		int32 RandomIndex = FMath::RandRange(1, AIDrop.Num() - 1);
        					GetWorld()->SpawnActor<ACollectableBox>(AIDrop[RandomIndex], T);
			        	}
			        }
        		}
        		else
        		{
        			UE_LOG(LogTemp, Display, TEXT("Anything else kill"));
        			GetWorld()->SpawnActor<ACollectableBox>(AIDrop[0], T);
        		}
        	}

            // Disable character movement
            GetCharacterMovement()->DisableMovement();
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

            // Stop AI logic
            if (AAIController* AICont = Cast<AAIController>(GetController()))
            {
                AICont->StopMovement();
                AICont->UnPossess();
            	AICont->Destroy();
            }

            // Enable ragdoll physics
        	if (!IsA(AFlyingEnemyAI::StaticClass()))
        	{
        		if (USkeletalMeshComponent* MeshComp = GetMesh())
        		{
        			// Disable any damage-causing components
        	

        			// Ragdoll physics setup
        			//MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
        			MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
        			MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        			MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        			MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        			MeshComp->SetGenerateOverlapEvents(false);

        			MeshComp->SetSimulatePhysics(true);
        			MeshComp->SetAllBodiesSimulatePhysics(true);
        			MeshComp->WakeAllRigidBodies();
        			MeshComp->bBlendPhysics = true;

        			FVector ImpulseDir = GetActorLocation() - DamageCauser->GetActorLocation();
        			ImpulseDir.Normalize();
        			MeshComp->AddImpulse(ImpulseDir * 100.0f, NAME_None, true);
        		}
        	//	Ensure all child components also ignore ECC_Pawn
	TArray<USceneComponent*> ChildComponents;
        		GetRootComponent()->GetChildrenComponents(true, ChildComponents);
        		for (USceneComponent* Child : ChildComponents)
        		{
        			if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Child))
        			{
        				// Only change if it's a collidable component
        				if (Primitive->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
        				{
        					Primitive->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        					Primitive->SetGenerateOverlapEvents(false); // Optional: avoid triggering damage overlaps
        				}
        			}
        		}
        	}

		    else
		    {
			    Destroy();
		    }
        	
        	// Optional: Destroy after some delay
        	SetLifeSpan(15.0f); // Character will be auto-destroyed after 10 seconds
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
		Move->AvoidanceConsiderationRadius = 300.f;
		Move->AvoidanceWeight              = 0.7f;
	}	//AI Crowd avoidance försök

	LastKnownLocation = GetActorLocation();
}

/* ─────────────────────────────────────────────── */
void AAI_Main::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	

	/*  Skippar om den är i luften */
	if (UCharacterMovementComponent* Move = GetCharacterMovement();
	    Move && (Move->IsFalling() || Move->IsFlying()))
	{
		return;
	}

	/*  Skippar “stuck” vid isfiring */
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
	if (FVector::DistSquared(Curr, LastKnownLocation) > MinMoveDistance * MinMoveDistance)
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
bool AAI_Main::IsOutsideNavMesh() const
{
	const UNavigationSystemV1* Nav =
	    FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Nav) return false;

	FNavLocation Dummy;
	return !Nav->ProjectPointToNavigation(GetActorLocation(), Dummy);
}
