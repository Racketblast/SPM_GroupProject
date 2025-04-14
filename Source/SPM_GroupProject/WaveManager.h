// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "WaveManager.generated.h"

USTRUCT(BlueprintType)
struct FWaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeBetweenSpawns;
};

//SPM_GROUPPROJECT_API

UCLASS()
class AWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AWaveManager();
	void TickGracePeriod();

protected:
	virtual void BeginPlay() override;

	void StartNextWave();
	void SpawnEnemy();
	void OnEnemyKilled(); // Call this from enemy class, when enemy dies

	void EndWave();

	UPROPERTY(EditAnywhere, Category = "Wave Config")
	TArray<FWaveData> Waves;

	UPROPERTY(EditAnywhere, Category = "Wave Config")
	TSubclassOf<AActor> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Wave Config")
	TArray<AActor*> SpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Wave Config")
	FWaveData DefaultWave;

	int32 CurrentWaveIndex;
	int32 EnemiesSpawnedInCurrentWave;
	int32 EnemiesKilledThisWave;
	
	FWaveData ActiveWaveData;

	FTimerHandle EnemySpawnTimer;

	// Grace period settings
	UPROPERTY(EditAnywhere, Category = "Wave Config")
	float GracePeriodDuration = 60.0f; // in seconds

	FTimerHandle GracePeriodTimer;
	int32 GraceSecondsRemaining;

	bool bIsGracePeriod = false;
};
