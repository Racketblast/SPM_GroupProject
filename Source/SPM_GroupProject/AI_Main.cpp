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
#include "FracturedLimbActor.h"
#include "Components/PoseableMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
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
    	// Extract hit bone and impact point if possible
    	FName BoneName = NAME_None;
    	FVector HitLocation = FVector::ZeroVector;

    	if (const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent))
    	{
    		BoneName = PointDamageEvent->HitInfo.BoneName;
    		HitLocation = PointDamageEvent->HitInfo.ImpactPoint;
    	}

    	if (GetMesh() && GetMesh()->IsSimulatingPhysics())
    	{
    		FVector TraceStart = DamageCauser->GetActorLocation();
    		FVector TraceEnd = HitLocation + (HitLocation - TraceStart) * 2;

    		FHitResult PhysicsHit;
    		FCollisionQueryParams Params;
    		Params.AddIgnoredActor(this);

    		bool bHitPhysics = GetWorld()->LineTraceSingleByChannel(
				PhysicsHit,
				TraceStart,
				TraceEnd,
				ECC_PhysicsBody,
				Params
			);

    		FName PhysicsBoneName = NAME_None;

    		if (bHitPhysics && PhysicsHit.Component.IsValid())
    		{
    			PhysicsBoneName = PhysicsHit.BoneName;

    			if (PhysicsBoneName == NAME_None)
    			{
    				USkeletalMeshComponent* HitMeshComp = Cast<USkeletalMeshComponent>(PhysicsHit.Component.Get());
    				if (HitMeshComp && HitMeshComp->GetPhysicsAsset())
    				{
    					int32 BodyIndex = HitMeshComp->GetPhysicsAsset()->FindBodyIndex(PhysicsBoneName);
    					if (BodyIndex == INDEX_NONE)
    					{
    						PhysicsBoneName = NAME_None;
    					}
    					else
    					{
    						PhysicsBoneName = HitMeshComp->GetPhysicsAsset()->SkeletalBodySetups[BodyIndex]->BoneName;
    					}
    				}
    			}
    		}


    		if (PhysicsBoneName != NAME_None)
    		{
    			HandleRagdollBoneHit(PhysicsBoneName, HitLocation, DamageAmount);
    			return DamageAmount;
    		}
    		else
    		{
    			AIHealth -= DamageAmount; // fallback
    		}
    	}
    	else
    	{
    		if (BoneName != NAME_None)
    		{
    			HandleBoneHit(BoneName, HitLocation, DamageAmount);
    		}
    		else
    		{
    			AIHealth -= DamageAmount;
    		}
    	}


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
        	// Disable capsule collision completely
        	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        	GetCapsuleComponent()->SetGenerateOverlapEvents(false);

        	// Disable collision on all child primitive components (like damage capsules)
        	TArray<USceneComponent*> ChildComponents;
        	GetRootComponent()->GetChildrenComponents(true, ChildComponents);

        	for (USceneComponent* Child : ChildComponents)
        	{
        		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Child))
        		{
        			Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        			Primitive->SetGenerateOverlapEvents(false);
        		}
        	}


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
        			HandleRagdollBoneHit(BoneName, HitLocation, DamageAmount);
        			BoneName = NAME_None;
        			HitLocation = FVector::ZeroVector;


        			if (const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent))
        			{
        				BoneName = PointDamageEvent->HitInfo.BoneName;
        				HitLocation = PointDamageEvent->HitInfo.ImpactPoint;
        			}

        			if (bIsDead && !IsProtectedBone(BoneName))
        			{
        				HandleRagdollBoneHit(BoneName, HitLocation, DamageAmount);
        			}


        		}
        	//	Ensure all child components also ignore ECC_Pawn

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
        	SetLifeSpan(25.0f); // Character will be auto-destroyed after 10 seconds
			}
        }
    

    return DamageAmount;
}
// Minor tweak in HandleRagdollBoneHit for physics blend weight:
void AAI_Main::HandleRagdollBoneHit(FName BoneName, FVector HitLocation, float DamageAmount)
{
    if (IsProtectedBone(BoneName)) return;
    if (IsA(AFlyingEnemyAI::StaticClass())) return;

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp || !MeshComp->DoesSocketExist(BoneName)) return;

    const float DismemberThreshold = 30.0f;
    const FName HeadBoneName = TEXT("head"); // Replace with your actual head bone name

    if (DamageAmount >= DismemberThreshold)
    {
        // Check if fractured limb for this bone already exists
        if (AttachedFracturedLimbs.Contains(BoneName))
        {
            // Already spawned fractured limb for this bone, skip
            return;
        }

        int32 BoneIndex = MeshComp->GetBoneIndex(BoneName);
        if (BoneIndex == INDEX_NONE) return;

        // Prevent dismembering head or its parents if alive
        if (AIHealth > 0.f)
        {
            int32 HeadIndex = MeshComp->GetBoneIndex(HeadBoneName);
            if (HeadIndex != INDEX_NONE && (MeshComp->BoneIsChildOf(HeadBoneName, BoneName) || BoneName == HeadBoneName))
            {
                return;
            }
        }

        // Spawn ragdoll fragment actor at the bone location
        FTransform BoneTransform = MeshComp->GetBoneTransform(BoneIndex);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AFracturedLimbActor* RagdollFragment = GetWorld()->SpawnActor<AFracturedLimbActor>(AFracturedLimbActor::StaticClass(), BoneTransform, SpawnParams);
        if (RagdollFragment)
        {
            // Setup skeletal mesh on the fragment
            USkeletalMeshComponent* SkeletalComp = RagdollFragment->FindComponentByClass<USkeletalMeshComponent>();
            if (!SkeletalComp)
            {
                // If the AFracturedLimbActor doesn’t already have one, add dynamically
                SkeletalComp = NewObject<USkeletalMeshComponent>(RagdollFragment);
                SkeletalComp->RegisterComponent();
                SkeletalComp->AttachToComponent(RagdollFragment->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
            }

            SkeletalComp->SetSkeletalMesh(MeshComp->SkeletalMesh);
            SkeletalComp->SetWorldTransform(BoneTransform);

            // Hide all bones except the fractured bone and its children
            int32 NumBones = SkeletalComp->GetNumBones();
            for (int32 i = 0; i < NumBones; ++i)
            {
                FName CurrentBone = SkeletalComp->GetBoneName(i);

                bool bKeepVisible = (CurrentBone == BoneName) || SkeletalComp->BoneIsChildOf(CurrentBone, BoneName);

                if (!bKeepVisible)
                {
                    SkeletalComp->HideBoneByName(CurrentBone, EPhysBodyOp::PBO_Term);
                }
            }

            // Setup physics for ragdoll fragment
            SkeletalComp->SetCollisionProfileName(TEXT("Ragdoll"));
            SkeletalComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SkeletalComp->SetSimulatePhysics(true);
            SkeletalComp->SetAllBodiesSimulatePhysics(true);
            SkeletalComp->WakeAllRigidBodies();
            SkeletalComp->bBlendPhysics = true;

            FVector ImpulseDir = (HitLocation - GetActorLocation()).GetSafeNormal();
            SkeletalComp->AddImpulseAtLocation(ImpulseDir * DamageAmount * 150.f, HitLocation);

            // Make sure SkeletalComp is root if none
            if (!RagdollFragment->GetRootComponent())
            {
                RagdollFragment->SetRootComponent(SkeletalComp);
            }

            // Store reference to manage fragments (optional)
            AttachedFracturedLimbs.Add(BoneName, RagdollFragment);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to spawn fractured limb actor"));
        }

        // Hide and disable physics on original mesh below fractured bone
        MeshComp->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
        MeshComp->SetAllBodiesBelowSimulatePhysics(BoneName, false, true);
        MeshComp->SetAllBodiesBelowPhysicsBlendWeight(BoneName, 0.0f, false, true);
    }
    else
    {
        // Just apply impulse if below threshold
        FVector Direction = (HitLocation - GetActorLocation()).GetSafeNormal();
        MeshComp->AddImpulseAtLocation(Direction * DamageAmount * 10.f, HitLocation, BoneName);
    }
}






/* ─────────────────────────────────────────────── */
void AAI_Main::BeginPlay()
{
	Super::BeginPlay();
	
	AIHealth = MaxAIHealth;
	InitializeLimbHealth();
	
	if (GetMesh())
	{
		LimbHealth.Empty();
		const int32 BoneCount = GetMesh()->GetNumBones();
		for (int32 i = 0; i < BoneCount; ++i)
		{
			FName BoneName = GetMesh()->GetBoneName(i);
			LimbHealth.Add(BoneName, DefaultLimbHealth);
		}
	}


	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bUseRVOAvoidance             = true;
		Move->AvoidanceConsiderationRadius = 300.f;
		Move->AvoidanceWeight              = 0.7f;
	}	//AI Crowd avoidance försök

	LastKnownLocation = GetActorLocation();
}
bool AAI_Main::IsProtectedBone(FName BoneName) const
{
	if (!GetMesh()) return false;

	FName RootBone = GetMesh()->GetBoneName(0);
	if (BoneName == RootBone)
	{
		return true;
	}

	// First child of root bone
	if (GetMesh()->GetNumBones() > 1)
	{
		FName FirstChild = GetMesh()->GetBoneName(1); // This assumes child index is 1, which is typical
		if (BoneName == FirstChild)
		{
			return true;
		}
	}

	return false;
}

void AAI_Main::InitializeLimbHealth()
{
	if (!GetMesh()) return;

	const int32 BoneCount = GetMesh()->GetNumBones();
	for (int32 i = 0; i < BoneCount; ++i)
	{
		FName BoneName = GetMesh()->GetBoneName(i);
		if (!LimbHealth.Contains(BoneName))
		{
			LimbHealth.Add(BoneName, DefaultLimbHealth);
		}
	}
}
// Add this in your AI_Main.h (or at top of cpp if you prefer quick fix):
// TMap to track fractured limbs per bone
TMap<FName, AFracturedLimbActor*> AttachedFracturedLimbs;


void AAI_Main::HandleBoneHit(FName BoneName, FVector HitLocation, float Damage)
{
	if (IsA(AFlyingEnemyAI::StaticClass()))
	{
		// You can optionally do something else here or just return
		return;
	}
	// apply local limb damage
	float& Health = LimbHealth[BoneName];
	Health -= Damage;

	if (Health <= 0.f)
	{
		// limb is "destroyed"
		HandleRagdollBoneHit(BoneName, HitLocation, Damage);
	}
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