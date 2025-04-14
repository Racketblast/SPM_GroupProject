// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Teleporter.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ATeleporter : public AActor
{
	GENERATED_BODY()
	
public:
	ATeleporter();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	FName TargetLevelName;
	
protected:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* CubeMeshComponent;


};
