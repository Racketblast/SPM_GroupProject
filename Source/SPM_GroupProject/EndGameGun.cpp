// Fill out your copyright notice in the Description page of Project Settings.


#include "EndGameGun.h"

#include "EndGameExplosive.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void AEndGameGun::Fire(FVector FireLocation, FRotator FireRotation)
{
    if (UGameplayStatics::GetCurrentLevelName(GetWorld(),true) == "Hub")
        return;
        
    if (bIsReloading)
        return;

    if (CurrentAmmo <= 0)
        return;

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
    if (ProjectileClass && GetWorld() && CurrentAmmo > 0)
    {
        AEndGameExplosive* Projectile = GetWorld()->SpawnActor<AEndGameExplosive>(ProjectileClass, FireLocation, FireRotation);
        if (Projectile)
        {
            Projectile->SetInstigator(Cast<APawn>(OwnerCharacter));  
        }
    }
    
    CurrentAmmo--;
    ApplyRecoilTranslation();
    
}

void AEndGameGun::ApplyRecoilTranslation()
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
            }, 0.35f, false); // 0.2 seconds delay
        }
    }
}