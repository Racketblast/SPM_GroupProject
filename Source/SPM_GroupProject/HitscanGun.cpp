#include "HitscanGun.h"
#include "WaveManager.h"
#include "EngineUtils.h"  // Needed for TActorIterator
#include "DrawDebugHelpers.h"
#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "UObject/UnrealType.h"
#include "WaveManager.h"
#include "Kismet/GameplayStatics.h"
void AHitscanGun::Fire(FVector FireLocation, FRotator FireRotation)
{
	if (bIsReloading)
	{
		return;
	}
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float TimeBetweenShots = 1.0f / RoundsPerSecond;

	// Example: Line trace or spawn projectile
	

	if (CurrentTime - LastFireTime < TimeBetweenShots || CurrentAmmo <= 0)
	{
		return; // Still in cooldown or no ammo
	}

	LastFireTime = CurrentTime;

	FVector ShotDirection = FireRotation.Vector();
	FVector End = FireLocation + (FireRotation.Vector() * Range);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (OwnerCharacter)
	{
		Params.AddIgnoredActor(OwnerCharacter);
	}

	if (GetWorld()->LineTraceSingleByChannel(Hit, FireLocation, End, ECC_Visibility, Params))
	{
		DrawDebugLine(GetWorld(), FireLocation, Hit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);

		AActor* HitActor = Hit.GetActor();
		LastHitActor = Hit.GetActor();
		if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
		{
			static const FName AIHealthName = TEXT("AIHealth");

			// Try to find the AIHealth int property
			if (FIntProperty* HealthProp = FindFProperty<FIntProperty>(HitCharacter->GetClass(), AIHealthName))
			{
				int32 CurrentHealth = HealthProp->GetPropertyValue_InContainer(HitCharacter);
				CurrentHealth -= static_cast<int32>(WeaponDamage); // Use this gun's damage value

				if (CurrentHealth <= 0)
				{
					//Should not spawn moneybox
					FActorSpawnParameters SpawnParams;
					TSubclassOf<AMoneyBox> MoneyBoxClass = LoadClass<AMoneyBox>(nullptr, TEXT("/Game/Blueprints/BP_MoneyBox.BP_MoneyBox_C"));
					if (MoneyBoxClass)
					{
						FTransform SpawnTransform = HitCharacter->GetTransform();
						FVector NewLocation = SpawnTransform.GetLocation();
						NewLocation.Z -= 100.0f;
						SpawnTransform.SetLocation(NewLocation);
						GetWorld()->SpawnActor<AMoneyBox>(MoneyBoxClass, SpawnTransform, SpawnParams);
					}
					HitCharacter->Destroy();

					// Find the WaveManager in the world and call OnEnemyKilled()
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
		DrawDebugLine(GetWorld(), FireLocation, End, FColor::Blue, false, 1.0f, 0, 1.0f);
	}

	CurrentAmmo--;
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
