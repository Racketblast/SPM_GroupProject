// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnemyAIUtils.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API UEnemyAIUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Försöker hitta en giltig teleporterings position nära spelaren, detta används av den flygande fienden
	UFUNCTION(BlueprintCallable, Category = "EnemyAI")
	static bool FindValidTeleportLocation(APawn* Pawn, const FVector& TargetLocation, FVector& OutLocation);

	UFUNCTION(BlueprintCallable, Category = "EnemyAI")
	static bool IsFlyableLocation(APawn* Pawn, UWorld* World, const FVector& Location, float ClearanceRadius = 100.f);
	
};
