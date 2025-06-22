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
#include "ZombieCharacter.h"
#include "GameFramework/DamageType.h"
#include "Engine/EngineTypes.h"
#include "Components/DecalComponent.h"
#include "AI_MAIN.h"
#include "FracturedLimbActor.h"


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
        Reload();

            return;
    }
    
    if (FireSound && FireAudioComponent)
    {
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
      
            {
                if (AAI_Main* AIEnemy = Cast<AAI_Main>(HitActor))


                {
                    FName BoneHit = Hit.BoneName;
                    FVector ImpactPoint = Hit.ImpactPoint;
                    AIEnemy->HandleBoneHit(BoneHit, ImpactPoint, WeaponDamage);
                }


            }
            if (AFracturedLimbActor* Limb = Cast<AFracturedLimbActor>(HitActor))
            {
                // Apply impulse and blood decal, but no damage
                if (Hit.Component.IsValid() && Hit.Component->IsSimulatingPhysics(Hit.BoneName))
                {
                    ApplyBloodDecal(Hit);

                    const float ImpulseStrength = 10000.0f;
                    FVector ImpulseDirection = (Hit.ImpactPoint - FireLocation).GetSafeNormal();
                    FVector Impulse = ImpulseDirection * ImpulseStrength;

                    Hit.Component->AddImpulseAtLocation(Impulse, Hit.ImpactPoint, Hit.BoneName);
                }
            }


            if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
            {
                if (APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerCharacter))
                {
                
                    // Apply damage if no custom function is found
                    float ActualDamage = WeaponDamage;
                    if (Hit.BoneName != NAME_None && Hit.BoneName.ToString().ToLower().Contains(TEXT("head")))
                    {
                        ActualDamage *= 2.0f;
                        UE_LOG(LogTemp, Warning, TEXT(" HEADSHOT! Applying double damage: %f"), ActualDamage);
                        Player->bEnemyHeadHit = true;
                        Player->EnemyHeadHitFalse();
                        UE_LOG(LogTemp, Error, TEXT("HEADSHOT CALLED"));
                    }
                    else
                    {
                        Player->bEnemyHit = true;
                        UE_LOG(LogTemp, Error, TEXT("hit activated (from gun)"));
                        Player->EnemyHitFalse();
                    }
                  

                    UGameplayStatics::ApplyPointDamage(
                        HitActor,
                        ActualDamage,
                        ShotDirection,
                        Hit,
                        OwnerCharacter ? OwnerCharacter->GetController() : nullptr,
                        this,
                        DamageType
                        );
                    if (Hit.Component.IsValid() && Hit.Component->IsSimulatingPhysics(Hit.BoneName))
                        {
                        ApplyBloodDecal(Hit);
                            const float ImpulseStrength = 10000.0f;
                            FVector ImpulseDirection = (Hit.ImpactPoint - FireLocation).GetSafeNormal(); // Direction of shot
                            FVector Impulse = ImpulseDirection * ImpulseStrength;

                            // Apply impulse at location on the specific bone
                            Hit.Component->AddImpulseAtLocation(Impulse, Hit.ImpactPoint, Hit.BoneName);

                            UE_LOG(LogTemp, Warning, TEXT("Applied impulse to bone: %s, Direction: %s"),
                                   *Hit.BoneName.ToString(),
                                   *ImpulseDirection.ToString());
                        }

                     
                    ApplyBloodDecal(Hit);
                
                } 

            }else {BulletHoleDecal(Hit);}
                }
                
              }
    if (OwnerCharacter)
    {
        float RecoilPitch = FMath::FRandRange(RecoilPitchMin, RecoilPitchMax);
        float RecoilYaw = FMath::FRandRange(RecoilYawMin, RecoilYawMax);

        OwnerCharacter->AddRecoilImpulse(FRotator(-RecoilPitch, RecoilYaw, 0.f)); // Negative pitch = up kick
        ApplyRecoilTranslation(); // Optional visual recoil
    }

    CurrentAmmo--;
    
}
