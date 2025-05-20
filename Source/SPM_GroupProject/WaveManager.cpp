
#include "WaveManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerGameInstance.h"
#include "TimerManager.h"
#include "MissionSubsystem.h"
#include "ChallengeSubsystem.h"
#include "FlyingEnemyAI.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

FWaveData AWaveManager::GenerateWaveData(int32 WaveIndex) const
{
	FWaveData ResultWave;

	if (Waves.IsValidIndex(WaveIndex))
	{
		ResultWave = Waves[WaveIndex];
	}
	else 
	{
		// DefaultWave
		ResultWave = DefaultWave;

		// Fallback för om man glömmer att fylla i DefaultWave i unreal.
		if (ResultWave.EnemyTypes.Num() == 0 && EnemyClass)
		{
			FEnemyTypeData DefaultType;
			DefaultType.EnemyClass = EnemyClass;
			DefaultType.MinCount = 5 + WaveIndex * 2;
			ResultWave.EnemyTypes.Add(DefaultType);
			ResultWave.MaxExtraCount = 3 + WaveIndex;
		}

		// Här skrivs koden som bestämmer hur svårt default wavesen ska vara. 
		for (FEnemyTypeData& Type : ResultWave.EnemyTypes)
		{
			Type.MinCount += (WaveIndex + 1) * DefaultWaveDifficultyMultiplier;
		}
		//ResultWave.MaxExtraCount += (WaveIndex + 1);
	}

	return ResultWave;
}


// Funktionen som startar waven. 
void AWaveManager::StartNextWave()
{
	// För challenges, den rensar challenge resultat status för nästa wave, detta är endast för widget. 
	/*if (UChallengeSubsystem* ChallengeSub = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
	{
		ChallengeSub->ResetChallengeStatus();
	}*/
	
	SpawnQueue.Empty();
	
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GI->bIsWave = true;
	}

	ActiveWaveData = GenerateWaveData(CurrentWaveIndex);

	// Lägger till fienderna som måste finnas med  (MinCount)
	for (const FEnemyTypeData& Type : ActiveWaveData.EnemyTypes)
	{
		for (int32 i = 0; i < Type.MinCount; ++i)
		{
			SpawnQueue.Add(Type.EnemyClass);
		}
	}

	// Lägger till slumpade extra fiender (MaxExtraCount)
	for (int32 i = 0; i < ActiveWaveData.MaxExtraCount; ++i)
	{
		int32 RandIndex = FMath::RandRange(0, ActiveWaveData.EnemyTypes.Num() - 1);
		SpawnQueue.Add(ActiveWaveData.EnemyTypes[RandIndex].EnemyClass);
	}

	// Blandar om Arrayn
	for (int32 i = SpawnQueue.Num() - 1; i > 0; --i)
	{
		int32 j = FMath::RandRange(0, i);
		SpawnQueue.Swap(i, j);
	}
	
	/*if (Waves.IsValidIndex(CurrentWaveIndex))
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
			Type.MinCount += (CurrentWaveIndex + 1)  * DefaultWaveDifficultyMultiplier;             // ökar minimum spawnas av varje enemy type
		}
		//ActiveWaveData.MaxExtraCount += (CurrentWaveIndex + 1);            // ökar maximum spawns
	}*/

	//För challenges
	if (UChallengeSubsystem* ChallengeSub = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
	{
		ChallengeSub->ActivateCurrentChallenge();
		ChallengeSub->StartWaveChallenge();
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

	UE_LOG(LogTemp, Warning, TEXT("This wave will spawn %d enemies."), TotalEnemiesToSpawn);
	
	/*GetWorldTimerManager().SetTimer(
		EnemySpawnTimer,
		this,
		&AWaveManager::SpawnEnemy,
		ActiveWaveData.TimeBetweenSpawns,
		true
	);*/
	SpawnEnemy();
}

// Denna funktion är den som faktisk spawnar in enemies 
void AWaveManager::SpawnEnemy()
{
	if (!SpawnQueue.IsValidIndex(EnemiesSpawnedInCurrentWave))
	{
		GetWorldTimerManager().ClearTimer(EnemySpawnTimer);
		return;
	}

	TSubclassOf<AActor> SelectedClass = SpawnQueue[EnemiesSpawnedInCurrentWave];

	// Filtrerar giltiga spawn points
	TArray<AEnemySpawnPoint*> ValidSpawnPoints;
	for (AEnemySpawnPoint* Point : SpawnPoints)
	{
		if (Point && Point->CanSpawn(SelectedClass))
		{
			ValidSpawnPoints.Add(Point);
		}
	}

	// Försöker igen ifall inga fungerande spawnpoints hittades 
	if (ValidSpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid spawn points for %s"), *SelectedClass->GetName());
		GetWorldTimerManager().SetTimerForNextTick(this, &AWaveManager::SpawnEnemy);
		return;
	}

	int32 RandomSpawnIndex = FMath::RandRange(0, ValidSpawnPoints.Num() - 1);
	FVector SpawnLocation = ValidSpawnPoints[RandomSpawnIndex]->GetActorLocation();

	SpawnedCountPerType.FindOrAdd(SelectedClass)++;
	EnemiesSpawnedInCurrentWave++;

	PlaySpawnVFXAndThenSpawnEnemy(SelectedClass, SpawnLocation);
	
	/*if (EnemiesSpawnedInCurrentWave >= TotalEnemiesToSpawn)
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
	
	
	// Här används EnemySpawnPoint klassen
	TArray<AEnemySpawnPoint*> ValidSpawnPoints;
	for (AEnemySpawnPoint* Point : SpawnPoints)
	{
		if (Point && Point->CanSpawn(SelectedClass))
		{
			ValidSpawnPoints.Add(Point);
		}
	}

	// Har gjort så att koden körs om och försöker spawna en fiende igen, ifall den misslyckas med att spawna en fiende. 
	if (ValidSpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid spawn points for %s"), *SelectedClass->GetName());
		GetWorldTimerManager().SetTimerForNextTick(this, &AWaveManager::SpawnEnemy);
		return;
	}

	int32 RandomSpawnIndex = FMath::RandRange(0, ValidSpawnPoints.Num() - 1);
	FVector SpawnLocation = ValidSpawnPoints[RandomSpawnIndex]->GetActorLocation();

	

	SpawnedCountPerType.FindOrAdd(SelectedClass)++;
	EnemiesSpawnedInCurrentWave++;

	PlaySpawnVFXAndThenSpawnEnemy(SelectedClass, SpawnLocation);*/
		
		
		/*FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(SelectedClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (SpawnedEnemy)
		{
			bSpawned = true;
			SpawnedCountPerType.FindOrAdd(SelectedClass)++;
			EnemiesSpawnedInCurrentWave++;

			// för flying enemy 
			if (AFlyingEnemyAI* FlyingEnemy = Cast<AFlyingEnemyAI>(SpawnedEnemy))
			{
				FlyingEnemy->SetMaxAltitude(MaxAltitude);
				FlyingEnemy->SetMinAltitude(MinAltitude);
				//UE_LOG(LogTemp, Warning, TEXT("FlyingEnemy MaxAltitude: %f"), MaxAltitude);
				//UE_LOG(LogTemp, Warning, TEXT("FlyingEnemy MinAltitude: %f"), MinAltitude);
			}
			
			UE_LOG(LogTemp, Warning, TEXT("Spawned enemy: %i"), EnemiesSpawnedInCurrentWave);
			break;
		}*/
}


void AWaveManager::PlaySpawnVFXAndThenSpawnEnemy(TSubclassOf<AActor> EnemyType, const FVector& SpawnLocation)
{
	if (!SpawnEffect) 
	{
		SpawnEnemyAtLocation(EnemyType, SpawnLocation); // ifall ingen vfx har blivit satt, så kallar den bara på spawn funktionen ändå
		return;
	}

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		SpawnEffect,
		SpawnLocation,
		FRotator::ZeroRotator,
		FVector(1.f),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true
	);

	if (NiagaraComp)
	{
		if (!NiagaraComp->OnSystemFinished.IsAlreadyBound(this, &AWaveManager::OnSpawnVFXFinished))
		{
			NiagaraComp->OnSystemFinished.AddDynamic(this, &AWaveManager::OnSpawnVFXFinished);
		}
		
		// Sparar stan, så man vet vilken fiende som ska bli spawnad efter att vfx är klar
		PendingSpawns.Add(NiagaraComp, EnemyType);
	}
}

void AWaveManager::OnSpawnVFXFinished(UNiagaraComponent* PSystem)
{
	if (!PSystem) return;

	if (PendingSpawns.Contains(PSystem))
	{
		TSubclassOf<AActor> EnemyType = PendingSpawns[PSystem];
		FVector SpawnLocation = PSystem->GetComponentLocation();
		PendingSpawns.Remove(PSystem);

		SpawnEnemyAtLocation(EnemyType, SpawnLocation);

		// Spawnar nästa fiende efter en kort delay
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AWaveManager::SpawnEnemy, ActiveWaveData.TimeBetweenSpawns, false);
	}
}

void AWaveManager::SpawnEnemyAtLocation(TSubclassOf<AActor> EnemyType, const FVector& SpawnLocation)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(EnemyType, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (SpawnedEnemy)
	{
		//SpawnedCountPerType.FindOrAdd(EnemyType)++;
		//EnemiesSpawnedInCurrentWave++;

		if (AFlyingEnemyAI* FlyingEnemy = Cast<AFlyingEnemyAI>(SpawnedEnemy))
		{
			FlyingEnemy->SetMaxAltitude(MaxAltitude);
			FlyingEnemy->SetMinAltitude(MinAltitude);
			//UE_LOG(LogTemp, Warning, TEXT("FlyingEnemy MaxAltitude: %f"), MaxAltitude);
			//UE_LOG(LogTemp, Warning, TEXT("FlyingEnemy MinAltitude: %f"), MinAltitude);
		}

		UE_LOG(LogTemp, Warning, TEXT("Spawned enemy after VFX: %i"), EnemiesSpawnedInCurrentWave);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn enemy after VFX."));
	}
}

// denna funktion kallas när fienden dör, vilket ska ske från enemy scriptet
void AWaveManager::OnEnemyKilled()
{
	EnemiesKilledThisWave++;
	UE_LOG(LogTemp, Warning, TEXT("Enemy died"));

	//För challenges
	if (UChallengeSubsystem* ChallengeSub = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
	{
		ChallengeSub->AddTimeToWaveChallenge();
	}
	
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
		// Challenges debuging
		if (UChallengeSubsystem* ChallengeSub = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
		{
			int32 f = ChallengeSub->GetCurrentChallengeRewardAmount();
		}
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
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GI->bIsWave = false;
	}
	
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

	PreviewNextWaveEnemyCount();

	// Challenges
	if (UChallengeSubsystem* ChallengeSub = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
	{
		ChallengeSub->CompleteCurrentChallenge();
		ChallengeSub->NotifyWaveCleared();
		ChallengeSub->PreviewNextChallenge();
	}

	// Missions
	if (UGameInstance* GI = GetGameInstance())
	{
		UMissionSubsystem* MissionSystem = GI->GetSubsystem<UMissionSubsystem>();
		if (MissionSystem)
		{
			MissionSystem->OnWaveCompleted(CurrentWaveIndex);
		}
	}
}


void AWaveManager::PreviewNextWaveEnemyCount()
{
	/*if (!Waves.IsValidIndex(CurrentWaveIndex + 1) && !DefaultWave.EnemyTypes.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid wave data to preview."));
		return;
	}*/

	int32 PreviewWaveIndex = CurrentWaveIndex + 1;

	FWaveData PreviewWave = GenerateWaveData(PreviewWaveIndex);

	int32 TotalEnemies = 0;
	for (const FEnemyTypeData& Type : PreviewWave.EnemyTypes)
	{
		TotalEnemies += Type.MinCount;
	}
	TotalEnemies += PreviewWave.MaxExtraCount;

	UE_LOG(LogTemp, Warning, TEXT("Next wave will spawn %d enemies."), TotalEnemies);

	// Lägger det i en variable för att sedan kunna vissa det i en widget
	UpcomingEnemyCount = TotalEnemies;
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



