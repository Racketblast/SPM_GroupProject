// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnPoint.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AEnemySpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawnPoint();

	// De fiender som får spawna från denna spawn point
	UPROPERTY(EditAnywhere, Category = "Spawn Restriction")
	TArray<TSubclassOf<AActor>> AllowedEnemyTypes;

	// Retunerar true ifall denna spawn point kan spawna den givna fiende typen. 
	bool CanSpawn(TSubclassOf<AActor> EnemyType) const;

};
