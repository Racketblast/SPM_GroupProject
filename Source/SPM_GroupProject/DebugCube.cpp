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

