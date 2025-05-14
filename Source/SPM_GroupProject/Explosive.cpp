#include "Explosive.h"
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "UObject/UnrealType.h"
#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "WaveManager.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"        
#include "Kismet/GameplayStatics.h"
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

    GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        ExplosionCenter,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(ExplosionRadius),
        Params
    );

    // VFX & SFX
    if (ExplosionEffectAsset)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffectAsset, ExplosionCenter, FRotator::ZeroRotator, true);
    }
    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, ExplosionCenter);
    }

    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* HitActor = Result.GetActor();
        if (!HitActor) continue;


        if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
        {
            // Optional: Visual feedback for player hit
            if (APlayerCharacter* Player = Cast<APlayerCharacter>(HitCharacter))
            {
                Player->bEnemyHit = true;
                UE_LOG(LogTemp, Warning, TEXT("Player hit by explosive"));
                Player->EnemyHitFalse();
            }

            // Construct HitResult (needed for ApplyPointDamage)
            FHitResult HitResult;
            HitResult.Location = HitCharacter->GetActorLocation();
            HitResult.ImpactPoint = HitResult.Location;
            HitResult.Component = Result.Component;

            UGameplayStatics::ApplyPointDamage(
                HitCharacter,
                WeaponDamage,
                (HitCharacter->GetActorLocation() - ExplosionCenter).GetSafeNormal(),
                HitResult,
                GetInstigatorController(),
                this,
                UDamageType::StaticClass()
            );
        }
        else if (HitActor->FindFunction("OnLineTraceHit"))
        {
            HitActor->ProcessEvent(HitActor->FindFunction("OnLineTraceHit"), nullptr);
        }
    }

    Destroy();
}
