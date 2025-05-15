#include "Shotgun.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"
#include "HitscanGun.h"
#include "WaveManager.h"
#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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
	{
		return;
	}

	if (CurrentAmmo <= 0)
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
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float TimeBetweenShots = 1.0f / RoundsPerSecond;

	if (CurrentTime - LastFireTime < TimeBetweenShots)
	{
		return;
	}

	LastFireTime = CurrentTime;
	

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
		{
			Params.AddIgnoredActor(OwnerCharacter);
		}

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
			//DrawDebugLine(GetWorld(), FireLocation, Hit.ImpactPoint, FColor::Red, false, 1.0f, 0, 0.5f);
			AActor* HitActor = Hit.GetActor();
			LastHitActor = HitActor;

			if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
			{
				bHitEnemyThisShot = true;

				static const FName AIHealthName = TEXT("AIHealth");
				if (FIntProperty* HealthProp = FindFProperty<FIntProperty>(HitCharacter->GetClass(), AIHealthName))
				{
					int32 CurrentHealth = HealthProp->GetPropertyValue_InContainer(HitCharacter);
					CurrentHealth -= static_cast<int32>(WeaponDamage / NumPellets);

					if (CurrentHealth <= 0)
					{
						TSubclassOf<AMoneyBox> MoneyBoxClass = LoadClass<AMoneyBox>(nullptr, TEXT("/Game/Blueprints/BP_MoneyBox.BP_MoneyBox_C"));
						if (MoneyBoxClass)
						{
							FTransform SpawnTransform = HitCharacter->GetTransform();
							FVector NewLocation = SpawnTransform.GetLocation();
							NewLocation.Z -= 100.0f;
							SpawnTransform.SetLocation(NewLocation);
							GetWorld()->SpawnActor<AMoneyBox>(MoneyBoxClass, SpawnTransform);
						}

						HitCharacter->Destroy();

						for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
						{
							if (AWaveManager* WaveManager = *It)
							{
								WaveManager->OnEnemyKilled();
								break;
							}
						}
					}
					else
					{
						HealthProp->SetPropertyValue_InContainer(HitCharacter, CurrentHealth);
					}
				}
			}
			else if (HitActor)
			{
				if (HitActor->FindFunction("OnLineTraceHit"))
				{
					HitActor->ProcessEvent(HitActor->FindFunction("OnLineTraceHit"), nullptr);
				}
			}
		}
		else
		{
			//DrawDebugLine(GetWorld(), FireLocation, End, FColor::Blue, false, 1.0f, 0, 0.5f);
		}
	}

	if (bHitEnemyThisShot)
	{
		bEnemyHit = true;
		UE_LOG(LogTemp, Error, TEXT("hit activated"));
		EnemyHitFalse();
	}

	CurrentAmmo--;
	ApplyRecoilTranslation();

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