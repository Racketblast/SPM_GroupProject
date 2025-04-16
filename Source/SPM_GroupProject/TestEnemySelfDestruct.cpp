// Fill out your copyright notice in the Description page of Project Settings.


#include "TestEnemySelfDestruct.h"
#include "EngineUtils.h"
#include "WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATestEnemySelfDestruct::ATestEnemySelfDestruct()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATestEnemySelfDestruct::BeginPlay()
{
	Super::BeginPlay();

	// Hitta wavemanager i leveln, viktig
	for (TActorIterator<AWaveManager> It(GetWorld()); It; ++It)
	{
		WaveManagerRef = *It;
		break; 
	}
	
	GetWorldTimerManager().SetTimer(SelfDestructTimer, this, &ATestEnemySelfDestruct::SelfDestruct, LifeTime, false);
}

void ATestEnemySelfDestruct::SelfDestruct()
{
	
	//Lägg detta i enemy när dem dör
	if (WaveManagerRef)
	{
		WaveManagerRef->OnEnemyKilled();
	}

	
	Destroy();
}


