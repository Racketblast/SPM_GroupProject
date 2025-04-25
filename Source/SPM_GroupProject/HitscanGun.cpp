#include "HitscanGun.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PlayerCharacter.h"
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
	}
	else
	{
		DrawDebugLine(GetWorld(), FireLocation, End, FColor::Blue, false, 1.0f, 0, 1.0f);
	}

	CurrentAmmo--;
}
