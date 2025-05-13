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

void AProjectileGun::Fire(FVector FireLocation, FRotator FireRotation)
{

	if (CurrentAmmo <= 0 &&bCanFire)
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
