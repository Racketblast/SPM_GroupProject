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
	// Sets default values for this actor's properties
	AEnemySpawnPoint();

	// De fiender som får spawna från denna spawn point
	UPROPERTY(EditAnywhere, Category = "Spawn Restriction")
	TArray<TSubclassOf<AActor>> AllowedEnemyTypes;

	// Retunerar true ifall denna spawn point kan spawna den givna fiende typen. 
	bool CanSpawn(TSubclassOf<AActor> EnemyType) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
