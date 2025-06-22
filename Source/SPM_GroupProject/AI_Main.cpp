#include "AI_Main.h"
#include "MoneyBox.h"
#include "Components/AudioComponent.h"
#include "WaveManager.h"
#include "AI_Controller.h"
#include "ChallengeSubsystem.h"
#include "PhysicsField/PhysicsFieldComponent.h"
#include "Field/FieldSystemObjects.h"
#include "Field/FieldSystemComponent.h"
#include "Chaos/ChaosSolverActor.h"

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
        	SetLifeSpan(60.0f); // Character will be auto-destroyed after 10 seconds
			}
        }
    

    return DamageAmount;
}
// Minor tweak in HandleRagdollBoneHit for physics blend weight:

void AAI_Main::SetInvisibleMaterialOnBoneSections(USkeletalMeshComponent* SkeletalComp, FName BoneName, UMaterialInterface* InvisibleMat)
{
	if (!SkeletalComp || !InvisibleMat) return;

	const USkeletalMesh* SkelMesh = SkeletalComp->GetSkeletalMeshAsset();
	if (!SkelMesh) return;

	const FSkeletalMeshRenderData* RenderData = SkelMesh->GetResourceForRendering();
	if (!RenderData || RenderData->LODRenderData.Num() == 0) return;

	const FSkeletalMeshLODRenderData& LODData = RenderData->LODRenderData[0];

	const TArray<FSkelMeshRenderSection>& Sections = LODData.RenderSections;
	for (int32 SectionIdx = 0; SectionIdx < Sections.Num(); ++SectionIdx)
	{
		const FSkelMeshRenderSection& Section = Sections[SectionIdx];
		if (Section.BoneMap.Contains(SkeletalComp->GetBoneIndex(BoneName)))
		{
			SkeletalComp->SetMaterial(SectionIdx, InvisibleMat);
		}
	}
}

void AAI_Main::HandleRagdollBoneHit(FName BoneName, FVector HitLocation, float DamageAmount)
{
    if (IsProtectedBone(BoneName) || IsA(AFlyingEnemyAI::StaticClass())) return;

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp || !MeshComp->DoesSocketExist(BoneName)) return;

    const float DismemberThreshold = 30.0f;
    const FName HeadBoneName = TEXT("Head");

    if (bIsHeadDestroyed && (BoneName == HeadBoneName || MeshComp->BoneIsChildOf(BoneName, HeadBoneName)))
        return;

    if (DamageAmount < DismemberThreshold)
    {
        FVector Impulse = (HitLocation - GetActorLocation()).GetSafeNormal() * DamageAmount * 10.f;
        MeshComp->AddImpulseAtLocation(Impulse, HitLocation, BoneName);
        return;
    }

    if (AttachedFracturedLimbs.Contains(BoneName)) return;

    if (BoneName == HeadBoneName)
    {
        if (AIHealth > 0) return;
        bIsHeadDestroyed = true;
    }
    else if (AIHealth > 0)
    {
        int32 HeadIndex = MeshComp->GetBoneIndex(HeadBoneName);
        if (HeadIndex != INDEX_NONE && (MeshComp->BoneIsChildOf(HeadBoneName, BoneName) || BoneName == HeadBoneName))
            return;
    }

    int32 BoneIndex = MeshComp->GetBoneIndex(BoneName);
    if (BoneIndex == INDEX_NONE) return;

    FTransform BoneTransform = MeshComp->GetBoneTransform(BoneIndex);
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AFracturedLimbActor* Fragment = GetWorld()->SpawnActor<AFracturedLimbActor>(AFracturedLimbActor::StaticClass(), BoneTransform, SpawnParams);
    if (!Fragment) return;

    Fragment->SetLifeSpan(25.f);

    USkeletalMeshComponent* FragMesh = NewObject<USkeletalMeshComponent>(Fragment);
    FragMesh->RegisterComponent();
    Fragment->SetRootComponent(FragMesh);

    FragMesh->SetSkeletalMeshAsset(MeshComp->GetSkeletalMeshAsset());
    FragMesh->SetWorldTransform(BoneTransform);
    SetInvisibleMaterialOnBoneSections(FragMesh, BoneName, InvisibleMaterial); // optional: hide shared material section

    FragMesh->SetCollisionProfileName(TEXT("Ragdoll"));
    FragMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    FragMesh->SetCollisionResponseToAllChannels(ECR_Block);
    FragMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    FragMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    TArray<USceneComponent*> ChildComps;
    Fragment->GetRootComponent()->GetChildrenComponents(true, ChildComps);
    for (USceneComponent* ChildComp : ChildComps)
    {
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ChildComp))
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            PrimComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        }
    }

    const USkeletalMesh* SkelMesh = FragMesh->GetSkeletalMeshAsset();
    const USkeleton* Skeleton = SkelMesh ? SkelMesh->GetSkeleton() : nullptr;
    if (!Skeleton) return;

    TSet<FName> BonesToKeep;
    FName CurrentBone = BoneName;
    while (true)
    {
        BonesToKeep.Add(CurrentBone);
        int32 CurrIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(CurrentBone);
        int32 ParentIndex = Skeleton->GetReferenceSkeleton().GetParentIndex(CurrIndex);
        if (ParentIndex == INDEX_NONE) break;
        CurrentBone = Skeleton->GetReferenceSkeleton().GetBoneName(ParentIndex);
    }

    TSet<FName> BonesToKeepWithDescendants = BonesToKeep;
    {
        const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
        TArray<int32> ToProcess;
        int32 HitBoneIndex = RefSkeleton.FindBoneIndex(BoneName);
        ToProcess.Add(HitBoneIndex);

        while (ToProcess.Num() > 0)
        {
            int32 CurrentIndex = ToProcess.Pop();
            FName CurrentBoneName = RefSkeleton.GetBoneName(CurrentIndex);

            if (bIsHeadDestroyed && (CurrentBoneName == HeadBoneName || FragMesh->BoneIsChildOf(CurrentBoneName, HeadBoneName)))
                continue;

            BonesToKeepWithDescendants.Add(CurrentBoneName);

            for (int32 i = 0; i < RefSkeleton.GetNum(); ++i)
            {
                if (RefSkeleton.GetParentIndex(i) == CurrentIndex)
                    ToProcess.Add(i);
            }
        }
    }

    int32 NumBones = FragMesh->GetNumBones();
    for (int32 i = 0; i < NumBones; ++i)
    {
        FName Bone = FragMesh->GetBoneName(i);
        if (!BonesToKeepWithDescendants.Contains(Bone))
        {
            FragMesh->HideBoneByName(Bone, EPhysBodyOp::PBO_Term);
        }
        else if (BonesToKeep.Contains(Bone) && !BonesToKeepWithDescendants.Contains(Bone))
        {
            FragMesh->HideBoneByName(Bone, EPhysBodyOp::PBO_None);
            FragMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            FragMesh->SetAllBodiesBelowSimulatePhysics(Bone, false, true);
            FragMesh->SetAllBodiesBelowPhysicsBlendWeight(Bone, 0.f, false, true);
        }
        else
        {
            FragMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            FragMesh->SetAllBodiesBelowSimulatePhysics(Bone, true, true);
            FragMesh->SetAllBodiesBelowPhysicsBlendWeight(Bone, 1.f, false, true);
        }
    }

    FragMesh->WakeAllRigidBodies();
    FragMesh->bBlendPhysics = true;

    FVector Impulse = (HitLocation - GetActorLocation()).GetSafeNormal() * DamageAmount * 150.f;
    FragMesh->AddImpulseAtLocation(Impulse, HitLocation);

    AttachedFracturedLimbs.Add(BoneName, Fragment);

    // Final: disable bone on original mesh
    MeshComp->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
    MeshComp->SetAllBodiesBelowSimulatePhysics(BoneName, false, true);
    MeshComp->SetAllBodiesBelowPhysicsBlendWeight(BoneName, 0.f, false, true);
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