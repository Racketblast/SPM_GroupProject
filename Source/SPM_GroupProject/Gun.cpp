// Gun.cpp
#include "Gun.h"
#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"

void AGun::Reload()
{
	UE_LOG(LogTemp, Warning, TEXT("Attempting reload. CurrentAmmo: %d, bHasInfiniteReloads: %d"), 
		   CurrentAmmo, bHasInfiniteReloads);

	if (bHasInfiniteReloads)
	{
		// Infinite reloads - reset the ammo to max value
		CurrentAmmo = MaxAmmo;
		UE_LOG(LogTemp, Warning, TEXT("Infinite reload: CurrentAmmo set to MaxAmmo: %d"), CurrentAmmo);
	}
	else if (TotalAmmo > 0)
	{
		int missingAmmo = MaxAmmo - CurrentAmmo;
		if (missingAmmo < TotalAmmo)
		{
			CurrentAmmo = MaxAmmo;
			TotalAmmo = TotalAmmo - missingAmmo;
		}
		else
		{
			CurrentAmmo = CurrentAmmo + TotalAmmo;
			TotalAmmo = 0;
		}
		// Regular reload (decrease extra mags, reset to max ammo)
		
		//CurrentAmmo = MaxAmmo;
		
	}
	else
	{
		// No extra mags left
		UE_LOG(LogTemp, Warning, TEXT("No extra mags left! Reload cannot be performed"));
	}

	// Final log to confirm ammo count after reload
	UE_LOG(LogTemp, Warning, TEXT("Reload complete. CurrentAmmo: %d"), CurrentAmmo);
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

