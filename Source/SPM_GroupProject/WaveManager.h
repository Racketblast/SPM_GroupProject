// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "Blueprint/UserWidget.h"
#include "WaveManager.generated.h"

// Detta är en struct som har data över hur en wave ser ut, man kan fylla i denna data i Unreal eller bara låta default waven ta hand om det.
//Då denna strcut låtter en customize en wave, kan man göra waves manuelt, med egna siffror upp till kanske wave 10, och sedan låta default waven ta hand om resten.
// Default waven SKA fyllas i! Annars blir spelet inte endless.
USTRUCT(BlueprintType)
struct FWaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeBetweenSpawns;
};

// Det finns viss data som måste fyllas i för att wave managern ska fungera. Detta kan du göra i unreal egine, där du behöver lägga till spawn points för fienderna att spawna på, samt se till att enemy class är i fylld med klassen som ska spawnas in.
// Default waven behöver också fyllas i. 
UCLASS()
class AWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AWaveManager();
	void OnEnemyKilled(); // Måste kalla på detta från enemy klassen, när enemy dör

protected:
	virtual void BeginPlay() override;

	void StartNextWave();
	void SpawnEnemy();
	void TickGracePeriod();
	void EndWave();

	//Widget
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentWaveNumber() const;

	UFUNCTION(BlueprintCallable)
	int32 GetEnemiesRemaining() const;

	// other
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

	// Settingsen för Grace period 
	UPROPERTY(EditAnywhere, Category = "Wave Config")
	float GracePeriodDuration = 60.0f; // i sekunder

	FTimerHandle GracePeriodTimer;
	int32 GraceSecondsRemaining;

	bool bIsGracePeriod = false;
};
