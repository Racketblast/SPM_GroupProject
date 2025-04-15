// Fill out your copyright notice in the Description page of Project Settings.


// Denna klass är bara for testing
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestEnemySelfDestruct.generated.h"

class AWaveManager;

UCLASS()
class ATestEnemySelfDestruct : public AActor
{
	GENERATED_BODY()
	
public:	
	ATestEnemySelfDestruct();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Self Destruct")
	float LifeTime = 5.0f;

	FTimerHandle SelfDestructTimer;

	// reference till WaveManager, viktigt
	AWaveManager* WaveManagerRef;

	void SelfDestruct();
};
