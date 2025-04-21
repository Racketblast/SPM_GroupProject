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

		// Fallback för om man glömmer att fylla i DefaultWave i unreal.
		if (ActiveWaveData.EnemyTypes.Num() == 0 && EnemyClass)
		{
			FEnemyTypeData DefaultType;
			DefaultType.EnemyClass = EnemyClass;
			DefaultType.MinCount = 5 + CurrentWaveIndex * 2;

			ActiveWaveData.EnemyTypes.Add(DefaultType);

			ActiveWaveData.MaxExtraCount = 3 + CurrentWaveIndex; // Detta är bara för att spawna extra fiender utöver de minimum kravet som MinCount sköter. Den behöver alltså inte vara större en MinCount och den bör faktiskt vara mindre egentligen för preformance skäl
		}

		// Här skrivs koden som bestämmer hur svårt default wavesen ska vara. Den utgår från det som skrivs in i unreal engine, och sedan adderas det med CurrentWaveIndex * DefaultWaveDifficultyMultiplier, detta kan dock ändras för balancing
		for (FEnemyTypeData& Type : ActiveWaveData.EnemyTypes)
		{
			Type.MinCount += (CurrentWaveIndex + 1) * DefaultWaveDifficultyMultiplier;             // ökar minimum spawnas av varje enemy type
			ActiveWaveData.MaxExtraCount += (CurrentWaveIndex + 1);            // ökar maximum spawns
		}
	}

	EnemiesSpawnedInCurrentWave = 0;
	EnemiesKilledThisWave = 0;
	SpawnedCountPerType.Empty();

	TotalEnemiesToSpawn = 0;
	for (const FEnemyTypeData& Type : ActiveWaveData.EnemyTypes)
	{
		TotalEnemiesToSpawn += Type.MinCount;
	}
	
	TotalEnemiesToSpawn += ActiveWaveData.MaxExtraCount;
	
	GetWorldTimerManager().SetTimer(
		EnemySpawnTimer,
		this,
		&AWaveManager::SpawnEnemy,
		ActiveWaveData.TimeBetweenSpawns,
		true
	);
}

// Denna funktion är den som faktisk spawnar in enemiesvoid AWaveManager::SpawnEnemy()
void AWaveManager::SpawnEnemy()
{
	if (EnemiesSpawnedInCurrentWave >= TotalEnemiesToSpawn)
	{
		GetWorldTimerManager().ClearTimer(EnemySpawnTimer);
		return;
	}

	const TArray<FEnemyTypeData>& EnemyTypes = ActiveWaveData.EnemyTypes; // is currently Waves[CurrentWaveIndex]. maybe should change to ActiveWaveData

	TSubclassOf<AActor> SelectedClass = nullptr;

	// Ser till att de minimum av alla fiender typer spawnas först. 
	for (const FEnemyTypeData& Type : EnemyTypes)
	{
		int32 Spawned = SpawnedCountPerType.FindRef(Type.EnemyClass);
		if (Spawned < Type.MinCount)
		{
			SelectedClass = Type.EnemyClass;
			break;
		}
	}

	// Ifall minimumet av alla fiender typer är spawnad, så slumpar den resten. 
	if (!SelectedClass)
	{
		int32 RandomIndex = FMath::RandRange(0, EnemyTypes.Num() - 1);
		SelectedClass = EnemyTypes[RandomIndex].EnemyClass;
	}

	// Har gjort så att koden körs om och försöker spawna en fiende igen, ifall den misslyckas med att spawna en fiende. 
	const int32 MaxAttempts = 5;
	bool bSpawned = false;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		int32 RandomSpawnIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		FVector SpawnLocation = SpawnPoints[RandomSpawnIndex]->GetActorLocation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(SelectedClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (SpawnedEnemy)
		{
			bSpawned = true;
			SpawnedCountPerType.FindOrAdd(SelectedClass)++;
			EnemiesSpawnedInCurrentWave++;
			UE_LOG(LogTemp, Warning, TEXT("Spawned enemy: %i"), EnemiesSpawnedInCurrentWave);
			break;
		}
	}

	if (!bSpawned)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn enemy after several attempts. Retrying..."));
		GetWorldTimerManager().SetTimerForNextTick(this, &AWaveManager::SpawnEnemy);
	}
}

// denna funktion kallas när fienden dör, vilket ska ske från enemy scriptet
void AWaveManager::OnEnemyKilled()
{
	EnemiesKilledThisWave++;
	UE_LOG(LogTemp, Warning, TEXT("Enemy died"));
	
	if (EnemiesKilledThisWave >= TotalEnemiesToSpawn)
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
	// Lägger till 1 eftersom att index börjar på 0, då CurrentWaveIndex är för en array
	return CurrentWaveIndex + 1;
}

int32 AWaveManager::GetEnemiesRemaining() const
{
	return TotalEnemiesToSpawn - EnemiesKilledThisWave;
}

float AWaveManager::GetGraceSecondsRemaining() const
{
	return GraceSecondsRemaining;
}

bool AWaveManager::IsInGracePeriod() const
{
	return GraceSecondsRemaining > 0.0f;
}



