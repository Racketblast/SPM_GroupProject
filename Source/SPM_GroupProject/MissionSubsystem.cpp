// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "PlayerGameInstance.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/Actor.h"


void UMissionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMissionSubsystem::OnWorldInitialized);
	
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem initialized."));
}

void UMissionSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem deinitialized."));
}

void UMissionSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
	NewMission(); // Nollställer variabler, så att man kan göra ett nytt mission.
	UE_LOG(LogTemp, Log, TEXT("New mission started for the new level!"));
}

void UMissionSubsystem::OnWaveCompleted(int32 WaveIndex)
{
	if (bIsCompleted) return; 

	WavesSurvived = WaveIndex + 1;

	if (WavesSurvived >= RequiredWaveToComplete)
	{
		CompleteMission();
	}
}

bool UMissionSubsystem::IsMissionCompleted() const
{
	return bIsCompleted;
}

void UMissionSubsystem::CompleteMission()
{
	bIsCompleted = true;
	UE_LOG(LogTemp, Warning, TEXT("Mission Complete! You survived %d waves."), WavesSurvived);

	// Level unlock 
	UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;
	
	FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
	GI->LastCompletedLevel = CurrentLevel;
	GI->UnlockedLevels.Add(CurrentLevel);

	int32 CurrentIndex = GI->LevelOrder.IndexOfByKey(CurrentLevel);
	if (CurrentIndex != INDEX_NONE && GI->LevelOrder.IsValidIndex(CurrentIndex + 1))
	{
		FName NextLevel = GI->LevelOrder[CurrentIndex + 1];
		GI->UnlockedLevels.Add(NextLevel);
		UE_LOG(LogTemp, Log, TEXT("Unlocked level: %s"), *NextLevel.ToString());
	}
}

void UMissionSubsystem::NewMission()
{
	bIsCompleted = false;
	WavesSurvived = 0;
}

void UMissionSubsystem::SetRequiredWavesToComplete(int32 NewRequired)
{
	RequiredWaveToComplete = NewRequired;
}


FText UMissionSubsystem::GetMissionStatusText() const
{
	if (bIsCompleted)
	{
		return FText::FromString(TEXT("Mission Complete!"));
	}

	return FText::Format(FText::FromString("Survive until Wave {0}"), FText::AsNumber(RequiredWaveToComplete));
}

