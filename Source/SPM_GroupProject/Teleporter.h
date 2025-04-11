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
	// Called every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	FName TargetLevelName = "Arena_1";

};
