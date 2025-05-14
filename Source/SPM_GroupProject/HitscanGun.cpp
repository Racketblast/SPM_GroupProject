#include "HitscanGun.h"
#include "WaveManager.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AI_Main.h"
#include "GameFramework/DamageType.h"
#include "Engine/EngineTypes.h"

void AHitscanGun::BeginPlay()
{
    Super::BeginPlay();

    if (!FireAudioComponent)
    {
        FireAudioComponent = NewObject<UAudioComponent>(this, TEXT("FireAudioComponent"));
        if (FireAudioComponent)
        {
            FireAudioComponent->RegisterComponent();
            FireAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
    if (!MagEmptyAudioComponent)
    {
        MagEmptyAudioComponent = NewObject<UAudioComponent>(this, TEXT("FireAudioComponent"));
        if (MagEmptyAudioComponent)
        {
            MagEmptyAudioComponent->RegisterComponent();
           MagEmptyAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
}
void AHitscanGun::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

  
}

void AHitscanGun::Fire(FVector FireLocation, FRotator FireRotation)
{
    if (bIsReloading)
    {
        return;
    }
   

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float TimeBetweenShots = 1.0f / RoundsPerSecond;

    if (CurrentTime - LastFireTime < TimeBetweenShots)
    {
        return;
    }

    LastFireTime = CurrentTime;
    if (CurrentAmmo <= 0)
    {
        if (MagEmptySound  && MagEmptyAudioComponent)
        {
            if (MagEmptyAudioComponent->IsPlaying())
            {
                MagEmptyAudioComponent->Stop();
            }
            MagEmptyAudioComponent->SetSound(MagEmptySound);
            MagEmptyAudioComponent->Play();
            return;
        }
    }
    if (FireSound && FireAudioComponent)
    {
        if (FireAudioComponent->IsPlaying())
        {
            FireAudioComponent->Stop();
        }
        FireAudioComponent->SetSound(FireSound);
        FireAudioComponent->Play();
    }

    if (MuzzleFlash && WeaponSkeletalMesh)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleFlash,
            WeaponSkeletalMesh,
            FName("MuzzleSocket"),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
    
    FVector ShotDirection = FireRotation.Vector();
    FVector End = FireLocation + (ShotDirection * Range);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (OwnerCharacter)
    {
        Params.AddIgnoredActor(OwnerCharacter);
    }

    if (GetWorld()->LineTraceSingleByChannel(Hit, FireLocation, End, ECC_Visibility, Params))
    {
        //DrawDebugLine(GetWorld(), FireLocation, Hit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);

        if (BulletHitEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                BulletHitEffect,
                Hit.Location
            );
        }
        AActor* HitActor = Hit.GetActor();
        LastHitActor = HitActor;

        if (AActor* HitCharacter = Cast<AActor>(HitActor))
        {
            if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerCharacter))
            {
                Player->bEnemyHit = true;
                UE_LOG(LogTemp, Error, TEXT("hit activated (from gun)"));
                Player->EnemyHitFalse();
            }


            if (HitActor)
            {
                if (HitActor->FindFunction("OnLineTraceHit"))
                {
                    HitActor->ProcessEvent(HitActor->FindFunction("OnLineTraceHit"), nullptr);
                }
                else
                {
                    // Apply damage if no custom function is found
                    UGameplayStatics::ApplyPointDamage(
                        HitActor,
                        WeaponDamage,
                        ShotDirection,
                        Hit,
                        OwnerCharacter ? OwnerCharacter->GetController() : nullptr,
                        this,
                        DamageType
                    );
                }
            }

        }}
    if (OwnerCharacter)
    {
        APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
        if (PC)
        {
            float RecoilPitch = FMath::FRandRange(RecoilPitchMin, RecoilPitchMax);
            float RecoilYaw = FMath::FRandRange(RecoilYawMin, RecoilYawMax);

            FRotator ControlRotation = PC->GetControlRotation();
            ControlRotation.Pitch -= RecoilPitch;
            ControlRotation.Yaw += RecoilYaw;

            PC->SetControlRotation(ControlRotation);
        }
    }
    CurrentAmmo--;
}