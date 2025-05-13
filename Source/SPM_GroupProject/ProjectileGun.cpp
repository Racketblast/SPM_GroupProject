#include "ProjectileGun.h"
#include "Projectile.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "PlayerCharacter.h"

void AProjectileGun::BeginPlay()
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
}

void AProjectileGun::Fire(FVector FireLocation, FRotator FireRotation)
{
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
			Projectile->SetInstigator(Cast<APawn>(OwnerCharacter));  // ✅ Set the instigator
		}

		CurrentAmmo--;
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
