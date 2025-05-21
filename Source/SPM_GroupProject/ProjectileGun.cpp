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
void AProjectileGun::ApplyRecoilTranslation()
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