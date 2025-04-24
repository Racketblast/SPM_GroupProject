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

	// Subscribe to the post-world initialization delegate
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMissionSubsystem::OnWorldInitialized);
	
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem initialized."));
}

void UMissionSubsystem::Deinitialize()
{
	Super::Deinitialize();

	// Unsubscribe from the delegate when the subsystem is deinitialized
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
	GI->Level += 1;
	if (GI->TeleportKeyArray.IsValidIndex(GI->Level))
	{
		GI->TeleportKeyArray[GI->Level] = true;
	}
}

void UMissionSubsystem::NewMission()
{
	bIsCompleted = false;
	WavesSurvived = 0;
}



FText UMissionSubsystem::GetMissionStatusText() const
{
	if (bIsCompleted)
	{
		return FText::FromString(TEXT("Mission Complete!"));
	}

	return FText::Format(NSLOCTEXT("Mission", "MissionStatus", "Survive until Wave {0}"), FText::AsNumber(RequiredWaveToComplete));
}

