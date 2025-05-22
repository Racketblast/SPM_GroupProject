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
	if (!MuzzleFlashLight)
	{
		MuzzleFlashLight = NewObject<UPointLightComponent>(this, TEXT("MuzzleFlashLight"));
		if (MuzzleFlashLight)
		{
			MuzzleFlashLight->Intensity = 400.0f;
			MuzzleFlashLight->SetVisibility(false);
			MuzzleFlashLight->bUseInverseSquaredFalloff = false;
			MuzzleFlashLight->LightColor = FColor::Orange;
			MuzzleFlashLight->AttenuationRadius = 300.0f;
			MuzzleFlashLight->RegisterComponent();

           
			MuzzleFlashLight->AttachToComponent(WeaponSkeletalMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("MuzzleSocket"));
		}
	}
}
void AProjectileGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PrimaryActorTick.bCanEverTick = true;

	if (bIsRecoveringFromRecoil)
	{
		RecoilRecoveryElapsed += DeltaTime;
		float Alpha = FMath::Clamp(RecoilRecoveryElapsed / RecoilRecoveryDuration, 0.0f, 1.0f);

		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OwnerCharacter);
		if (PlayerCharacter && PlayerCharacter->ArmsRoot)
		{
			FVector NewLocation = FMath::Lerp(RecoilStartLocation, RecoilTargetLocation, Alpha);
			PlayerCharacter->ArmsRoot->SetRelativeLocation(NewLocation);
		}

		if (Alpha >= 1.0f)
		{
			bIsRecoveringFromRecoil = false;
			bRecoilApplied = false;
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
		if (MuzzleFlashLight)
		{
			MuzzleFlashLight->SetVisibility(true);

			// Automatically hide it shortly after (e.g. 0.05s)
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				if (MuzzleFlashLight)
				{
					MuzzleFlashLight->SetVisibility(false);
				}
			}, 0.05f, false);
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
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OwnerCharacter);
	if (PlayerCharacter && PlayerCharacter->ArmsRoot)
	{
		FVector BackwardDirection = -PlayerCharacter->GetActorForwardVector();

		if (!bRecoilApplied)
		{
			bRecoilApplied = true;
			OriginalArmsRootLocation = PlayerCharacter->ArmsRoot->GetRelativeLocation();

			// Apply recoil instantly
			FVector RecoilTranslation = BackwardDirection * RecoilAmount;
			PlayerCharacter->ArmsRoot->AddWorldOffset(RecoilTranslation);

			// Set up interpolation recovery
			RecoilStartLocation = PlayerCharacter->ArmsRoot->GetRelativeLocation();
			RecoilTargetLocation = OriginalArmsRootLocation;
			RecoilRecoveryElapsed = 0.0f;
			bIsRecoveringFromRecoil = true;
		}
	}
}