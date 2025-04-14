// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AWaveManager::AWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;

	CurrentWaveIndex = 0;
	EnemiesSpawnedInCurrentWave = 0;
	EnemiesKilledThisWave = 0;
}

void AWaveManager::BeginPlay()
{
	Super::BeginPlay();

	StartNextWave();
}

void AWaveManager::StartNextWave()
{
	if (Waves.IsValidIndex(CurrentWaveIndex))
	{
		ActiveWaveData = Waves[CurrentWaveIndex];
	}
	else
	{
		ActiveWaveData = DefaultWave;

		// Optional: make the game more difficult 
		ActiveWaveData.NumEnemies += CurrentWaveIndex * 2;
	}

	EnemiesSpawnedInCurrentWave = 0;
	EnemiesKilledThisWave = 0;

	GetWorldTimerManager().SetTimer(
		EnemySpawnTimer,
		this,
		&AWaveManager::SpawnEnemy,
		ActiveWaveData.TimeBetweenSpawns,
		true
	);
}

void AWaveManager::SpawnEnemy()
{
	if (EnemiesSpawnedInCurrentWave >= ActiveWaveData.NumEnemies)
	{
		GetWorldTimerManager().ClearTimer(EnemySpawnTimer);
		return;
	}

	if (EnemyClass && SpawnPoints.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		FVector RandomSpawnLocation = SpawnPoints[RandomIndex]->GetActorLocation();

		FActorSpawnParameters SpawnParams;
		GetWorld()->SpawnActor<AActor>(EnemyClass, RandomSpawnLocation, FRotator::ZeroRotator, SpawnParams);

		EnemiesSpawnedInCurrentWave++;
	}
}

void AWaveManager::OnEnemyKilled()
{
	EnemiesKilledThisWave++;

	if (EnemiesKilledThisWave >= ActiveWaveData.NumEnemies)
	{
		EndWave();
	}
}

void AWaveManager::TickGracePeriod()
{
	if (GraceSecondsRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(GracePeriodTimer);
		bIsGracePeriod = false;
		CurrentWaveIndex++;

		UE_LOG(LogTemp, Warning, TEXT("Grace period ended. Starting next wave..."));
		StartNextWave();
		return;
	}

	// Log to screen
	GEngine->AddOnScreenDebugMessage(
		-1,
		1.1f,
		FColor::Green,
		FString::Printf(TEXT("Next wave in: %d seconds"), GraceSecondsRemaining)
	);

	GraceSecondsRemaining--;
}


void AWaveManager::EndWave()
{
	bIsGracePeriod = true;
	GraceSecondsRemaining = FMath::CeilToInt(GracePeriodDuration);

	// Start countdown tick every second
	GetWorldTimerManager().SetTimer(
		GracePeriodTimer,
		this,
		&AWaveManager::TickGracePeriod,
		1.0f,
		true
	);

	UE_LOG(LogTemp, Warning, TEXT("Grace period started: %d seconds"), GraceSecondsRemaining);
}






