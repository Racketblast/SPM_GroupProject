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

void AShotgun::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<APlayerCharacter>(GetOwner());

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

void AShotgun::Fire(FVector FireLocation, FRotator FireRotation)
{
	if (bIsReloading)
		return;

	if (CurrentAmmo <= 0)
	{
		if (MagEmptySound && MagEmptyAudioComponent)
		{
			if (MagEmptyAudioComponent->IsPlaying())
				MagEmptyAudioComponent->Stop();

			MagEmptyAudioComponent->SetSound(MagEmptySound);
			MagEmptyAudioComponent->Play();
		}
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

					ApplyBloodDecalTemp(Hit);
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
void AShotgun::ApplyRecoilTranslation()
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
			}, 0.25f, false); // 0.2 seconds delay
		}
	}
}
void AShotgun::ApplyBloodDecalTemp(const FHitResult& Hit)
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

void AShotgun::BulletHoleDecal(const FHitResult& Hit)
{
	FVector SurfaceNormal = Hit.Normal;
	FRotator DecalRotation = SurfaceNormal.Rotation();

	UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		BulletDecalMaterial,
		FVector(10.0f, 10.0f, 10.0f),
		Hit.Location,
		DecalRotation,
		60.0f
	);
}
