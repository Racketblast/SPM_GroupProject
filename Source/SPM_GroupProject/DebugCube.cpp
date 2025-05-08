// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugCube.h"

#include "PlayerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/SGameLayerManager.h"

// Sets default values
ADebugCube::ADebugCube()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());

}

void ADebugCube::Use_Implementation(APlayerCharacter* Player)
{
	DoAllFunctions();
}

void ADebugCube::DoAllFunctions()
{
	if (UseSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, UseSound, GetActorLocation());
	}
	EnableAllLevels();
	GivePlayerMaxHealth();
	GivePlayerMaxMoney();
	GiveAllWeapons();
}

void ADebugCube::EnableAllLevels()
{
	if (bLevel)
	{
		if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			
			if (GI->LevelOrder.Num() > 0)
			{
				GI->UnlockedLevels.Add(GI->LevelOrder[0]);
				GI->UnlockedLevels.Add(GI->LevelOrder[1]);
				GI->UnlockedLevels.Add(GI->LevelOrder[2]);
				GI->UnlockedLevels.Add(GI->LevelOrder[3]);
				GI->UnlockedLevels.Add(GI->LevelOrder[4]);
			}
		}
	}
}

void ADebugCube::GivePlayerMaxHealth()
{
	if (bHealth)
	{
		if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			GI->BuyUpgrade(EUpgradeType::HealthMax);
		}
	}
}

void ADebugCube::GivePlayerMaxMoney()
{
	if (bMoney)
	{
		if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			GI->Money += 1000000;
		}
	}
}

void ADebugCube::GiveAllWeapons()
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (bUnlockAllWeapons)
		{
			if (!GI->UpgradeMap.Contains(EUpgradeType::Pistol))
			{
				GI->UpgradeMap.Add(EUpgradeType::Pistol, GI->SetDefaultUpgradeInfo(EUpgradeType::Pistol));
				GI->SetCurrentWeapon(EUpgradeType::Pistol);
			}
			// Rifle
			if (!GI->UpgradeMap.Contains(EUpgradeType::Rifle))
			{
				GI->UpgradeMap.Add(EUpgradeType::Rifle, GI->SetDefaultUpgradeInfo(EUpgradeType::Rifle));
			}

			// Shotgun
			if (!GI->UpgradeMap.Contains(EUpgradeType::Shotgun))
			{
				GI->UpgradeMap.Add(EUpgradeType::Shotgun, GI->SetDefaultUpgradeInfo(EUpgradeType::Shotgun));
			}

			// Rocket Launcher
			if (!GI->UpgradeMap.Contains(EUpgradeType::RocketLauncher))
			{
				GI->UpgradeMap.Add(EUpgradeType::RocketLauncher, GI->SetDefaultUpgradeInfo(EUpgradeType::RocketLauncher));
			}
		}
	}
	
}
