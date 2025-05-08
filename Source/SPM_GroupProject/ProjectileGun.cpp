// ProjectileGun.cpp
#include "ProjectileGun.h"
#include "Projectile.h"

void AProjectileGun::Fire(FVector FireLocation, FRotator FireRotation)
{
	if (ProjectileClass && GetWorld() &&CurrentAmmo > 0)
	{
	
		GetWorld()->SpawnActor<AExplosive>(ProjectileClass, FireLocation, FireRotation);
		CurrentAmmo--;
		UE_LOG(LogTemp, Warning, TEXT("Fired! Current Ammo: %d"), CurrentAmmo);
	}

}