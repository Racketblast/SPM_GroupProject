// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawnPoint.h"

// Sets default values
AEnemySpawnPoint::AEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemySpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AEnemySpawnPoint::CanSpawn(TSubclassOf<AActor> EnemyType) const
{
	return AllowedEnemyTypes.Num() == 0 || AllowedEnemyTypes.Contains(EnemyType);
}

