// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionManager.h"

void UMissionManager::Initialize(int32 WavesToSurvive)
{
	RequiredWaves = WavesToSurvive;
	WavesSurvived = 0;
	bIsCompleted = false;
}

void UMissionManager::OnWaveCompleted(int32 WaveIndex)
{
	if (bIsCompleted) return;

	WavesSurvived = WaveIndex + 1;

	if (WavesSurvived >= RequiredWaves)
	{
		CompleteMission();
	}
}

void UMissionManager::CompleteMission()
{
	bIsCompleted = true;
	OnMissionCompleted.Broadcast();
}

bool UMissionManager::IsMissionCompleted() const
{
	return bIsCompleted;
}

int32 UMissionManager::GetRequiredWaves() const
{
	return RequiredWaves;
}

int32 UMissionManager::GetWavesSurvived() const
{
	return WavesSurvived;
}