// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AFlyingEnemyAI;

class SPM_GROUPPROJECT_API FlyingAIHelper
{
public:
	static FVector CalculateTargetLocation(
		UWorld* World,
		ACharacter* Player,
		AFlyingEnemyAI* FlyingEnemy,
		const FVector& FromLocation,
		float ZOffset,
		bool bAddRandomOffset,
		float RandomRadius,
		float ObstacleCheckDistance,
		float ObstacleClearance
	);

	static bool IsLocationClear(
		UWorld* World,
		const FVector& Location,
		AActor* IgnoredActor,
		float Distance,
		float Clearance
	);

	// Används inte just nu
	static bool IsPathClear(UWorld* World,
	const FVector& FromLocation,
	const FVector& ToLocation,
	AFlyingEnemyAI* FlyingEnemy); 
};
