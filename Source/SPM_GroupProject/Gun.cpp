// Gun.cpp
#include "Gun.h"
#include "PlayerCharacter.h"

void AGun::Reload()
{
	UE_LOG(LogTemp, Warning, TEXT("Attempting reload. CurrentAmmo: %d, ExtraMags: %d, bHasInfiniteReloads: %d"), 
		   CurrentAmmo, ExtraMags, bHasInfiniteReloads);

	if (bHasInfiniteReloads)
	{
		// Infinite reloads - reset the ammo to max value
		CurrentAmmo = MaxAmmo;
		UE_LOG(LogTemp, Warning, TEXT("Infinite reload: CurrentAmmo set to MaxAmmo: %d"), CurrentAmmo);
	}
	else if (ExtraMags > 0)
	{
		// Regular reload (decrease extra mags, reset to max ammo)
		ExtraMags--;
		CurrentAmmo = MaxAmmo;
		UE_LOG(LogTemp, Warning, TEXT("Regular reload: ExtraMags decremented. CurrentAmmo set to MaxAmmo: %d, Remaining ExtraMags: %d"), 
			   CurrentAmmo, ExtraMags);
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

