// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveManager.h"
#include "Blueprint/UserWidget.h"
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

// Funktionen som startar waven. 
void AWaveManager::StartNextWave()
{
	if (Waves.IsValidIndex(CurrentWaveIndex))
	{
		ActiveWaveData = Waves[CurrentWaveIndex];
	}
	else
	{
		ActiveWaveData = DefaultWave;

		// Här skrivs koden som bestämmer hur svårt default wavesen ska vara. Den utgår från det som skrivs in i unreal engine, och sedan adderas det med CurrentWaveIndex * 2, detta kan dock ändras för balancing
		ActiveWaveData.NumEnemies += (CurrentWaveIndex + 1) * 2;
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

// Denna funktion är den som faktisk spawnar in enemies
void AWaveManager::SpawnEnemy()
{
	if (EnemiesSpawnedInCurrentWave >= ActiveWaveData.NumEnemies)
	{
		GetWorldTimerManager().ClearTimer(EnemySpawnTimer);
		return;
	}

	if (!EnemyClass || SpawnPoints.Num() == 0) return;

	const int32 MaxAttempts = 5;
	bool bSpawned = false;
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		FVector SpawnLocation = SpawnPoints[RandomIndex]->GetActorLocation();
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; 

		AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (SpawnedEnemy)
		{
			bSpawned = true;
			EnemiesSpawnedInCurrentWave++;
			UE_LOG(LogTemp, Warning, TEXT("Spawned enemy: %i"), EnemiesSpawnedInCurrentWave);
			break;
		}
	}

	if (!bSpawned)
	{
		// denna del kanske är onödig, har aldrig sätt medelandet
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn enemy after several attempts. Retrying..."));

		GetWorldTimerManager().SetTimerForNextTick(this, &AWaveManager::SpawnEnemy);
	}
}

// denna funktion kallas när fienden dör, vilket ska ske från enemy scriptet
void AWaveManager::OnEnemyKilled()
{
	EnemiesKilledThisWave++;
	UE_LOG(LogTemp, Warning, TEXT("Enemy died"));
	
	if (EnemiesKilledThisWave >= ActiveWaveData.NumEnemies)
	{
		EndWave();
	}
}

// funktionen som gör att det tar x sekunder innan nästa wave startar
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

	// Skriv ut GraceSecondsRemaining direkt till skärmen. Använder nu en Widget istället, dock är den 1 sekund fel, men tror det är ok
	/*GEngine->AddOnScreenDebugMessage(
		-1,
		1.1f,
		FColor::Green,
		FString::Printf(TEXT("Next wave in: %d seconds"), GraceSecondsRemaining)
	);*/

	GraceSecondsRemaining--;
}

// funktionen som sätter igång TickGracePeriod när alla enemies är döda, kallas från OnEnemyKilled funktionen
void AWaveManager::EndWave()
{
	bIsGracePeriod = true;
	GraceSecondsRemaining = FMath::CeilToInt(GracePeriodDuration);
	
	GetWorldTimerManager().SetTimer(
		GracePeriodTimer,
		this,
		&AWaveManager::TickGracePeriod,
		1.0f,
		true
	);

	UE_LOG(LogTemp, Warning, TEXT("Grace period started: %d seconds"), GraceSecondsRemaining);
}

int32 AWaveManager::GetCurrentWaveNumber() const
{
	// Add 1 since index starts at 0
	return CurrentWaveIndex + 1;
}

int32 AWaveManager::GetEnemiesRemaining() const
{
	return ActiveWaveData.NumEnemies - EnemiesKilledThisWave;
}

float AWaveManager::GetGraceSecondsRemaining() const
{
	return GraceSecondsRemaining;
}

bool AWaveManager::IsInGracePeriod() const
{
	return GraceSecondsRemaining > 0.0f;
}



