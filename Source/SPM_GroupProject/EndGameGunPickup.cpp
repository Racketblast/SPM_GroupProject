// Fill out your copyright notice in the Description page of Project Settings.


#include "EndGameGunPickup.h"

#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEndGameGunPickup::AEndGameGunPickup()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
}

void AEndGameGunPickup::Use_Implementation(APlayerCharacter* Player)
{
	if (!Player->bCanSwitchWeapons)
	return;
	
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!GI->UpgradeMap.Contains(EUpgradeType::DoomsdayGun))
		{
			GI->UpgradeMap.Add(EUpgradeType::DoomsdayGun);
			Player->SelectWeapon("DoomsdayGun");
		}
	}
	Destroy();
}

// Called when the game starts or when spawned
void AEndGameGunPickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (GI->CurrentGameFlag < 1)
		{
			Destroy();
		}
		else if (GI->UpgradeMap.Contains(EUpgradeType::DoomsdayGun))
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0)))
			{
				GI->UpgradeMap.Remove(EUpgradeType::DoomsdayGun);
				GI->CurrentWeapon = EUpgradeType::Pistol;
			}
		}
	}
}
