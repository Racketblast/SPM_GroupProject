// Gun.cpp
#include "Gun.h"
#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"


AGun::AGun()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create components using *this*, since we're inside the constructor
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);  // Make WeaponMesh the root

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(WeaponMesh);  // Attach muzzle to weapon

	// Optional: Forward offset of 50 units
	MuzzlePoint->SetRelativeLocation(FVector(50.f, 0.f, 0.f));
}



void AGun::Reload()
{
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload already in progress!"));
		return;
	}

	if (CurrentAmmo == MaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ammo already full."));
		return;
	}

	if (!bHasInfiniteReloads && TotalAmmo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No extra mags left! Reload cannot be performed."));
		return;
	}

	bIsReloading = true;

	UE_LOG(LogTemp, Warning, TEXT("Reload started..."));

	// Start a 2-second timer to finish the reload
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &AGun::FinishReload, 2.0f, false);
}
void AGun::FinishReload()
{
	if (bHasInfiniteReloads)
	{
		CurrentAmmo = MaxAmmo;
	}
	else
	{
		int missingAmmo = MaxAmmo - CurrentAmmo;
		if (missingAmmo <= TotalAmmo)
		{
			CurrentAmmo = MaxAmmo;
			TotalAmmo -= missingAmmo;
		}
		else
		{
			CurrentAmmo += TotalAmmo;
			TotalAmmo = 0;
		}
	}

	bIsReloading = false;

	UE_LOG(LogTemp, Warning, TEXT("Reload complete. CurrentAmmo: %d, TotalAmmo: %d"), CurrentAmmo, TotalAmmo);
}




void AGun::SetOwnerCharacter(APlayerCharacter* NewOwner)
{
	OwnerCharacter = NewOwner;
}

void AGun::CheckForUpgrades()
{
	if (bHasAppliedUpgrades) return;
	
	if (!bIsUpgraded)
	{
		if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(),0)))
			{
				for (EUpgradeType Upgrades : GI->UpgradeArray)
				{
					GI->UpgradeGunStats(Upgrades, Player);
				}
			}
		}
		bHasAppliedUpgrades = true;
	}
}

