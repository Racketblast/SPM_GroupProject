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

    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* HitActor = Result.GetActor();
        if (!HitActor) continue;

        UPrimitiveComponent* HitComponent = Result.GetComponent();
        if (!HitComponent) continue;

        FVector TargetLocation = HitActor->GetActorLocation();
        float Distance = FVector::Dist(ExplosionCenter, TargetLocation);

        // Skip anything outside radius just in case
        if (Distance > ExplosionRadius)
            continue;

        // Scale factor (1 = center, 0 = edge)
        float DistanceScale = 1.0f - (Distance / ExplosionRadius);
        DistanceScale = FMath::Clamp(DistanceScale, 0.0f, 1.0f);

        float ScaledDamage = WeaponDamage * DistanceScale;
        float ScaledPushForce = ExplosionPushForce * DistanceScale;

        FVector Direction = (TargetLocation - ExplosionCenter).GetSafeNormal();
        FVector LaunchVelocity = Direction * ScaledPushForce;
        LaunchVelocity.Z += ScaledPushForce * 0.5f;  // Add upward kick

        if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
        {
            HitCharacter->LaunchCharacter(LaunchVelocity, true, true);

            FHitResult HitResult;
            HitResult.Location = HitCharacter->GetActorLocation();
            HitResult.Component = HitComponent;

            UGameplayStatics::ApplyPointDamage(
                HitCharacter,
                ScaledDamage,
                Direction,
                HitResult,
                GetInstigatorController(),
                this,
                UDamageType::StaticClass()
            );

            if (APlayerCharacter* Player = Cast<APlayerCharacter>(HitCharacter))
            {
                Player->bEnemyHit = true;
                Player->EnemyHitFalse();
                UE_LOG(LogTemp, Warning, TEXT("Player hit by explosive"));
            }
        }
        else
        {
            // If it's a physics object, apply impulse
            if (HitComponent->IsSimulatingPhysics())
            {
                HitComponent->AddImpulse(LaunchVelocity, NAME_None, true);
            }

            // Optional: trigger Blueprintable explosion behavior
            if (HitActor->FindFunction("OnLineTraceHit"))
            {
                HitActor->ProcessEvent(HitActor->FindFunction("OnLineTraceHit"), nullptr);
            }
        }
    }

    Destroy();
}
