#include "ProjectileGun.h"
#include "Projectile.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "PlayerCharacter.h"




void AProjectileGun::Fire(FVector FireLocation, FRotator FireRotation)
{

	if (CurrentAmmo <= 0)
	{
		Reload();
		/*if (MagEmptySound  && MagEmptyAudioComponent)
		{
			if (MagEmptyAudioComponent->IsPlaying())
			{
				MagEmptyAudioComponent->Stop();
			}
			MagEmptyAudioComponent->SetSound(MagEmptySound);
			MagEmptyAudioComponent->Play();
		}*/
		return;
	}
	if (!bCanFire || bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot fire: either reloading or waiting for fire rate cooldown."));
		return;
	}



	if (ProjectileClass && GetWorld() && CurrentAmmo > 0)
	{
		AExplosive* Projectile = GetWorld()->SpawnActor<AExplosive>(ProjectileClass, FireLocation, FireRotation);
		if (Projectile)
		{
			Projectile->SetInstigator(Cast<APawn>(OwnerCharacter));  
		}
		if (OwnerCharacter)
		{
			float RecoilPitch = FMath::FRandRange(RecoilPitchMin, RecoilPitchMax);
			float RecoilYaw = FMath::FRandRange(RecoilYawMin, RecoilYawMax);

			OwnerCharacter->AddRecoilImpulse(FRotator(-RecoilPitch, RecoilYaw, 0.f)); // Negative pitch = up kick
			ApplyRecoilTranslation(); 
		}
		CurrentAmmo--;
		//ApplyRecoilTranslation();
		UE_LOG(LogTemp, Warning, TEXT("Fired! Current Ammo: %d"), CurrentAmmo);

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
		bCanFire = false;
		float FireCooldown = 1.0f / RoundsPerSecond;
		GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AProjectileGun::ResetFire, FireCooldown, false);
	}
}


void AProjectileGun::ResetFire()
{
	bCanFire = true;
}
