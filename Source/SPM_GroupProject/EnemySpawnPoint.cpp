// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawnPoint.h"


AEnemySpawnPoint::AEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Retunerar true ifall denna spawn point kan spawna den givna fiende typen. 
bool AEnemySpawnPoint::CanSpawn(TSubclassOf<AActor> EnemyType) const
{
	return AllowedEnemyTypes.Num() == 0 || AllowedEnemyTypes.Contains(EnemyType);
}

