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
#include "Components/DecalComponent.h"
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
        MagEmptyAudioComponent = NewObject<UAudioComponent>(this, TEXT("MagEmptyAudioComponent"));
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
        
        if (HitActor)
        {
            if (HitActor->FindFunction("OnLineTraceHit"))
            {
                HitActor->ProcessEvent(HitActor->FindFunction("OnLineTraceHit"), nullptr);
            }
            if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
            {
                if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerCharacter))
                {
                    Player->bEnemyHit = true;
                    UE_LOG(LogTemp, Error, TEXT("hit activated (from gun)"));
                    Player->EnemyHitFalse();
                }
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
                ApplyBloodDecal(Hit); // Replace values as needed

                
            } else {BulletHoleDecal(Hit);}

        }}
    if (OwnerCharacter)
    {
        float RecoilPitch = FMath::FRandRange(RecoilPitchMin, RecoilPitchMax);
        float RecoilYaw = FMath::FRandRange(RecoilYawMin, RecoilYawMax);

        OwnerCharacter->AddRecoilImpulse(FRotator(-RecoilPitch, RecoilYaw, 0.f)); // Negative pitch = up kick
        ApplyRecoilTranslation(); // Optional visual recoil
    }

    CurrentAmmo--;
    
}
void AHitscanGun::ApplyRecoilTranslation()
{
    // Check if we have a valid reference to the player character and the ArmsRoot component
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OwnerCharacter);
    if (PlayerCharacter && PlayerCharacter->ArmsRoot)
    {
        // Get the direction the player is facing in world space (backward direction from player)
        FVector BackwardDirection = -PlayerCharacter->GetActorForwardVector(); // This points backward in world space

        // Check if recoil has been applied already, to avoid applying it multiple times
        if (!bRecoilApplied)
        {
            // Store the original position of the arms root before applying recoil
            OriginalArmsRootLocation = PlayerCharacter->ArmsRoot->GetRelativeLocation();

            // Apply recoil translation in world space (backward toward the player)
            FVector RecoilTranslation = BackwardDirection * RecoilAmount; // Recoil backwards in world space
            PlayerCharacter->ArmsRoot->AddWorldOffset(RecoilTranslation); // Apply the offset in world space

            // Set the recoil flag to true so we don't apply it again
            bRecoilApplied = true;

            // Set up a timer to return the arms root to the original position after 0.2 seconds
            FTimerHandle TimerHandle;

            // Set the timer to move the arms back to the original position after 0.2 seconds
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [PlayerCharacter, this]()
            {
                // Reset the arms to the original position
                if (PlayerCharacter && PlayerCharacter->ArmsRoot)
                {
                    PlayerCharacter->ArmsRoot->SetRelativeLocation(OriginalArmsRootLocation);
                }

                // After resetting, allow recoil to be applied again
                bRecoilApplied = false;
            }, 0.15f, false); // 0.2 seconds delay
        }
    }
}

void AHitscanGun::ApplyBloodDecal(const FHitResult& Hit)
{
    if (!BloodDecalMaterial) return;

    AActor* HitActor = Hit.GetActor();
    if (!HitActor) return;

    USkeletalMeshComponent* SkeletalMesh = HitActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh) return;

    FVector DecalSize = FVector(20.0f, 20.0f, 20.0f);
    FRotator DecalRotation = Hit.Normal.Rotation();

    // Get bone name only if valid
    FName BoneName = Hit.BoneName != NAME_None ? Hit.BoneName : NAME_None;

    // Attach decal to the skeletal mesh, to a bone if available
    UDecalComponent* BloodDecal = UGameplayStatics::SpawnDecalAttached(
        BloodDecalMaterial,
        DecalSize,
        SkeletalMesh,
        BoneName,
        Hit.ImpactPoint,
        DecalRotation,
        EAttachLocation::KeepWorldPosition,
        60.0f // lifespan
    );

    if (BloodDecal)
    {
        BloodDecal->SetFadeScreenSize(0.001f);
        UE_LOG(LogTemp, Warning, TEXT("Spawned decal on skeletal mesh bone: %s"), *BoneName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn decal"));
    }
}




void AHitscanGun::BulletHoleDecal(const FHitResult& Hit)
{
    FVector SurfaceNormal = Hit.Normal; // Normal of the hit surface (floor in this case)

    FRotator DecalRotation = SurfaceNormal.Rotation(); // Convert the surface normal to a rotation

    // Spawn the decal at the hit location with the correct rotation and scale
    UGameplayStatics::SpawnDecalAtLocation(
        GetWorld(),
        BulletDecalMaterial,  // The material
        FVector(10.0f, 10.0f, 10.0f), // Decal size (adjust as needed)
        Hit.Location,  // Location where the bullet hit
        DecalRotation,  // Adjusted rotation
        60.0f  // Decal lifetime (adjust as needed)
    );
}
