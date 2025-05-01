// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugCube.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ADebugCube : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADebugCube();

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bLevel = false;
	void EnableAllLevels();
protected:
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* StaticMeshComponent;
};
