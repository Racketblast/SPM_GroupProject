#include "Shotgun.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "HitscanGun.h"
#include "PlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h" // Required for UDecalComponent


void AShotgun::Fire(FVector FireLocation, FRotator FireRotation)
{
	if (bIsReloading)
		return;

	if (CurrentAmmo <= 0)
	{
		Reload();
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float TimeBetweenShots = 1.0f / RoundsPerSecond;

	if (CurrentTime - LastFireTime < TimeBetweenShots)
		return;

	LastFireTime = CurrentTime;

	if (FireSound && FireAudioComponent)
	{
		if (FireAudioComponent->IsPlaying())
			FireAudioComponent->Stop();

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

	bool bHitEnemyThisShot = false;

	for (int32 i = 0; i < NumPellets; i++)
	{
		FRotator PelletRotation = FireRotation;
		PelletRotation.Pitch += FMath::FRandRange(-SpreadAngle, SpreadAngle);
		PelletRotation.Yaw += FMath::FRandRange(-SpreadAngle, SpreadAngle);

		FVector ShotDirection = PelletRotation.Vector();
		FVector End = FireLocation + (ShotDirection * Range);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (OwnerCharacter)
			Params.AddIgnoredActor(OwnerCharacter);

		if (GetWorld()->LineTraceSingleByChannel(Hit, FireLocation, End, ECC_Visibility, Params))
		{
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
						Player->EnemyHitFalse();
					}

					// Apply point damage per pellet
					UGameplayStatics::ApplyPointDamage(
						HitActor,
						WeaponDamage / NumPellets,
						ShotDirection,
						Hit,
						OwnerCharacter ? OwnerCharacter->GetController() : nullptr,
						this,
						DamageType
					);

					ApplyBloodDecal(Hit);
					bHitEnemyThisShot = true;
				}
				else
				{
					BulletHoleDecal(Hit);
				}
			}
		}
	}

	if (bHitEnemyThisShot)
	{
		bEnemyHit = true;
		EnemyHitFalse();
	}

	CurrentAmmo--;

	if (OwnerCharacter)
	{
		float RecoilPitch = FMath::FRandRange(RecoilPitchMin, RecoilPitchMax);
		float RecoilYaw = FMath::FRandRange(RecoilYawMin, RecoilYawMax);

		OwnerCharacter->AddRecoilImpulse(FRotator(-RecoilPitch, RecoilYaw, 0.f));
		ApplyRecoilTranslation();
	}
}


void AShotgun::EnemyHitFalse()
{
	bEnemyHit = false;
	UE_LOG(LogTemp, Error, TEXT("hit false"));
}



