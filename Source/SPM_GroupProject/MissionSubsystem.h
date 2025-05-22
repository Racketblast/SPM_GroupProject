// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MissionSubsystem.generated.h"


UCLASS()
class SPM_GROUPPROJECT_API UMissionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void OnWaveCompleted(int32 WaveIndex);
	void NewMission();
	bool IsMissionCompleted() const;
	void TryUnlockLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void SetRequiredWavesToComplete(int32 NewRequired);

	// för widget
	UFUNCTION(BlueprintCallable, Category = "Mission")
	FText GetMissionStatusText() const;
	
	int32 RequiredWaveToComplete = 2;
	int32 WavesSurvived = 0;
protected:
	
private:
	bool bIsCompleted = false;

	void CompleteMission();
	
	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS);
};
