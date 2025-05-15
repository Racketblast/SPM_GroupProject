// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoBox.h"

#include "Gun.h"
#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

void AAmmoBox::CollectableBoxTriggeredFunction(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			if (AGun* Weapon = Player->GetWeaponInstance(ToWeapon))
			{
				if (Weapon->TotalAmmo < Weapon->MaxTotalAmmo)
				{
					UE_LOG(LogTemp, Warning, TEXT("AmmoBoxTriggered"));
					Weapon->TotalAmmo += Weapon->MaxTotalAmmo/5;
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollectablePickUpSound, GetActorLocation());
				}
			}
			Destroy();
		}
	}
}
