#include "Explosive.h"
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "UObject/UnrealType.h"
#include "PlayerCharacter.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"        
#include "Engine/EngineTypes.h"               
#include "FlyingEnemyAI.h"

AExplosive::AExplosive()
{
	PrimaryActorTick.bCanEverTick = false;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>("ExplosiveMesh");
	RootComponent = ExplosiveMesh;

	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));
	ProjectileComponent->InitialSpeed = 1200.f;
	ProjectileComponent->bShouldBounce = true;
}

void AExplosive::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(ExplosionTimer, this, &AExplosive::Explode, ExplosionDelay, false);
}





void AExplosive::Explode()
{
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    FVector ExplosionCenter = GetActorLocation();

    // Sphere overlap to detect everything around
    GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        ExplosionCenter,
        FQuat::Identity,
        ECC_WorldDynamic,
        FCollisionShape::MakeSphere(ExplosionRadius),
        Params
    );

    // VFX & SFX
    if (ExplosionEffectAsset)
    {
        float ScaleFactor = ExplosionRadius / 200.0f; // Adjust 100.0f if your particle system expects a different base size

        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ExplosionEffectAsset,
            ExplosionCenter,
            FRotator::ZeroRotator,
            FVector(ScaleFactor) // Uniform scale based on radius
        );
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, ExplosionCenter);
    }

   TSet<AActor*> DamagedActors;

    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* HitActor = Result.GetActor();
        if (!HitActor || DamagedActors.Contains(HitActor))
            continue;

        DamagedActors.Add(HitActor);

        UPrimitiveComponent* HitComponent = Result.GetComponent();
        if (!HitComponent) continue;

        FVector TargetLocation = HitActor->GetActorLocation();
        float Distance = FVector::Dist(GetActorLocation(), TargetLocation);

        if (Distance > ExplosionRadius)
            continue;

        float DistanceScale = 1.0f - (Distance / ExplosionRadius);
        if (DistanceScale > 0.6)
        {
            DistanceScale = 1.0f;
        }
        DistanceScale = FMath::Clamp(DistanceScale, 0.0f, 1.0f);

        float ScaledDamage = ExplosionDamage * DistanceScale;
        float ScaledPushForce = ExplosionPushForce * DistanceScale;

        FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
        FVector LaunchVelocity = Direction * ScaledPushForce;
        LaunchVelocity.Z += ScaledPushForce * 0.5f;

        if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
        {
            if (!HitCharacter->IsA<AFlyingEnemyAI>())
            {
                HitCharacter->LaunchCharacter(LaunchVelocity, true, true);
            }
           

            FHitResult HitResult;
            HitResult.Location = HitCharacter->GetActorLocation();
            HitResult.Component = HitComponent;
            
            if (APlayerCharacter* Player = Cast<APlayerCharacter>(HitCharacter))
            {
                Player->bEnemyHit = true;
                Player->EnemyHitFalse();
                UE_LOG(LogTemp, Warning, TEXT("Player hit by explosive"));
                UGameplayStatics::ApplyPointDamage(
                    HitCharacter,
                    ScaledDamage/4,
                    Direction,
                    HitResult,
                    GetInstigatorController(),
                    this,
                    UDamageType::StaticClass()
                );
            }
            else
            {
                UGameplayStatics::ApplyPointDamage(
                    HitCharacter,
                    ScaledDamage,
                    Direction,
                    HitResult,
                    GetInstigatorController(),
                    this,
                    UDamageType::StaticClass()
                );
            }
        }
        else
        {
            if (HitComponent->IsSimulatingPhysics())
            {
                HitComponent->AddImpulse(LaunchVelocity, NAME_None, true);
            }

            if (HitActor->FindFunction("OnLineTraceHit"))
            {
                HitActor->ProcessEvent(HitActor->FindFunction("OnLineTraceHit"), nullptr);
            }
        }
    }


    Destroy();   
}



