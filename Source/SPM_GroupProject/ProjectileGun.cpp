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
		return;
	}
	if (!bCanFire || bIsReloading)
	{
		return;
	}


	if (ProjectileClass && GetWorld() && CurrentAmmo > 0)
	{
		AExplosive* Projectile = GetWorld()->SpawnActor<AExplosive>(ProjectileClass, FireLocation, FireRotation);
		Projectile->ExplosionDamage = WeaponDamage;
		if (Projectile)
		{
			Projectile->SetInstigator(Cast<APawn>(OwnerCharacter));  
		}
		if (OwnerCharacter)
		{
			ApplyRecoilTranslation(); 
		}
		CurrentAmmo--;
		//ApplyRecoilTranslation();
		UE_LOG(LogTemp, Warning, TEXT("Fired! Current Ammo: %d"), CurrentAmmo);

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
		bCanFire = false;
		float FireCooldown = 1.0f / RoundsPerSecond;
		GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AProjectileGun::ResetFire, FireCooldown, false);
	}
}


void AProjectileGun::ResetFire()
{
	bCanFire = true;
}
