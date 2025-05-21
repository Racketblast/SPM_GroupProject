// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "Blueprint/UserWidget.h"
#include "EnemySpawnPoint.h"
#include "NiagaraSystem.h"
#include "WaveManager.generated.h"


//Struct för en fiende typ, här kan man ställa in hur många av just denna typ av fiende som ska spawnas under en wave.
USTRUCT(BlueprintType)
struct FEnemyTypeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCount = 0;
};

// Detta är en struct som har data över hur en wave ser ut, man kan fylla i denna data i Unreal eller bara låta default waven ta hand om det.
//Då denna strcut låtter en customize en wave, kan man göra waves manuelt, med egna siffror upp till kanske wave 10, och sedan låta default waven ta hand om resten.
// Default waven SKA fyllas i! Annars blir spelet inte endless.
USTRUCT(BlueprintType)
struct FWaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemyTypeData> EnemyTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeBetweenSpawns = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxExtraCount = 0;
};

USTRUCT()
struct FPendingEnemySpawnData
{
	GENERATED_BODY()

	TSubclassOf<AActor> EnemyClass;
	FVector SpawnLocation;

	FPendingEnemySpawnData() {}
	FPendingEnemySpawnData(TSubclassOf<AActor> InClass, FVector InLocation)
		: EnemyClass(InClass), SpawnLocation(InLocation) {}
};

// Det finns viss data som måste fyllas i för att wave managern ska fungera. Detta kan du göra i unreal egine, där du behöver lägga till spawn points för fienderna att spawna på, samt se till att enemy class är i fylld med klassen som ska spawnas in.
// Default waven behöver också fyllas i. 
UCLASS()
class SPM_GROUPPROJECT_API AWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AWaveManager();
	
	UFUNCTION(BlueprintCallable)
	void OnEnemyKilled(); // Måste kalla på detta från enemy klassen, när enemy dör

	// För widgets, så man kan få antalet fiender som kommer nästa wave
	UFUNCTION(BlueprintCallable)
	int32 GetUpcomingEnemyCount() const { return UpcomingEnemyCount; }

protected:
	virtual void BeginPlay() override;

	void StartNextWave();
	void SpawnEnemy();
	void TickGracePeriod();
	void EndWave();
	void PreviewNextWaveEnemyCount();

	UFUNCTION()
	FWaveData GenerateWaveData(int32 WaveIndex) const;

	UPROPERTY()
	TMap<UNiagaraComponent*, FPendingEnemySpawnData> PendingSpawns;
	
	TMap<UNiagaraComponent*, TSubclassOf<AActor>> PendingSpawnQueue;

	//Widget
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentWaveNumber() const;

	UFUNCTION(BlueprintCallable)
	int32 GetEnemiesRemaining() const;

	UFUNCTION(BlueprintCallable, Category = "Wave")
	float GetGraceSecondsRemaining() const;

	UFUNCTION(BlueprintCallable, Category = "Wave")
	bool IsInGracePeriod() const;

	// other
	UPROPERTY(EditAnywhere, Category = "Wave Config")
	TArray<FWaveData> Waves;

	UPROPERTY(EditAnywhere, Category = "Wave Config") // finns endast kvar för fallbacken i StartNextWave funktionen
	TSubclassOf<AActor> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<AEnemySpawnPoint*> SpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Wave Config")
	FWaveData DefaultWave;

	UPROPERTY(EditAnywhere, Category = "Wave Config")
	int32 DefaultWaveDifficultyMultiplier = 2;

	UPROPERTY(EditAnywhere, Category = "Spawning") 
	int32 EnemiesPerSpawnBatch = 2;

	/*UPROPERTY(EditAnywhere, Category = "Spawning")
	int32 MaxConcurrentSpawnVFX = 2;*/

	int32 CurrentWaveIndex;
	
	TMap<TSubclassOf<AActor>, int32> SpawnedCountPerType;
	int32 TotalEnemiesToSpawn;
	int32 EnemiesSpawnedInCurrentWave;
	int32 EnemiesKilledThisWave;
	
	FWaveData ActiveWaveData;

	FTimerHandle EnemySpawnTimer;

	UPROPERTY()
	TArray<TSubclassOf<AActor>> SpawnQueue;


	// Grace perioden för innan den första waven börjar.  
	FTimerHandle FirstWaveGraceTimer;

	UPROPERTY(EditAnywhere, Category = "Wave Config")
	float FirstWaveTimer = 5.0f;

	int32 FirstGraceSecondsRemaining;

	bool bIsFirstGracePeriod = false;

	void UpdateFirstWaveCountdown();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	float GetFirstGraceSecondsRemaining() const { return FirstGraceSecondsRemaining; }

	UFUNCTION(BlueprintCallable, Category = "Wave")
	bool IsFirstGracePeriod() const { return bIsFirstGracePeriod; }

	// Settingsen för Grace period 
	UPROPERTY(EditAnywhere, Category = "Wave Config")
	float GracePeriodDuration = 60.0f; // i sekunder

	FTimerHandle GracePeriodTimer;
	
	int32 GraceSecondsRemaining;

	bool bIsGracePeriod = false;

	// for the Flying enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlyingEnemy")
	float MaxAltitude = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FlyingEnemy")
	float MinAltitude = 100.0f;

	// Upcoming enemies
	int32 UpcomingEnemyCount = 0;

	//För vfx
	UPROPERTY(EditAnywhere, Category="Spawning")
	UNiagaraSystem* SpawnEffect;

	void PlaySpawnVFXAndThenSpawnEnemy(TSubclassOf<AActor> EnemyClass, const FVector& SpawnLocation);

	UFUNCTION()
	void OnSpawnVFXFinished(UNiagaraComponent* PSystem);

	void SpawnEnemyAtLocation(TSubclassOf<AActor> EnemyClass, const FVector& SpawnLocation);

	int32 PendingVFXSpawns = 0;

	TQueue<FPendingEnemySpawnData> SpawnVFXQueue;

	int32 ActiveVFXCount = 0;
	
	bool bIsSpawningEnemy = false; 	// Håller kåll på om en VFX spelas upp just nu. 

	void HandleNextSpawnInQueue();

};
