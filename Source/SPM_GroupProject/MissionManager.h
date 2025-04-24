// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MissionManager.generated.h"

// Delegate to notify when a mission is completed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionCompleted);

/**
 * 
 */
UCLASS(Blueprintable)
class SPM_GROUPPROJECT_API UMissionManager : public UObject
{
	GENERATED_BODY()

public:
	// Initialize mission with how many waves the player must survive
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void Initialize(int32 WavesToSurvive);

	// Call this each time a wave is completed
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void OnWaveCompleted(int32 WaveIndex);

	// Check if mission is already completed
	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool IsMissionCompleted() const;

	// Delegate to broadcast when mission is complete
	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FOnMissionCompleted OnMissionCompleted;

	// Get total waves needed for UI display
	UFUNCTION(BlueprintCallable, Category = "Mission")
	int32 GetRequiredWaves() const;

	// Get how many waves player has survived
	UFUNCTION(BlueprintCallable, Category = "Mission")
	int32 GetWavesSurvived() const;

private:
	UPROPERTY()
	int32 RequiredWaves = 0;

	UPROPERTY()
	int32 WavesSurvived = 0;

	UPROPERTY()
	bool bIsCompleted = false;

	void CompleteMission();
	
};
