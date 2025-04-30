#include "WaveManager.h"
#include "EngineUtils.h"  // Needed for TActorIterator
#include "HitscanGun.h"
#include "DrawDebugHelpers.h"
#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "UObject/UnrealType.h"
#include "WaveManager.h"
#include "Kismet/GameplayStatics.h"

void AHitscanGun::Fire(FVector FireLocation, FRotator FireRotation)
{
	if (CurrentAmmo <= 0) return;

	FVector ShotDirection = FireRotation.Vector();
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
		DrawDebugLine(GetWorld(), FireLocation, Hit.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);

		AActor* HitActor = Hit.GetActor();
		if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
		{
			static const FName AIHealthName = TEXT("AIHealth");

			// Try to find the AIHealth int property
			if (FIntProperty* HealthProp = FindFProperty<FIntProperty>(HitCharacter->GetClass(), AIHealthName))
			{
				int32 CurrentHealth = HealthProp->GetPropertyValue_InContainer(HitCharacter);
				CurrentHealth -= static_cast<int32>(Damage); // Use this gun's damage value

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
	}
	else
	{
		DrawDebugLine(GetWorld(), FireLocation, End, FColor::Blue, false, 1.0f, 0, 1.0f);
	}

	CurrentAmmo--;
}


