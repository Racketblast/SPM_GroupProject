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
	WavesSurvived = WaveIndex + 1;
	
	if (bIsCompleted) return; 

	//UE_LOG(LogTemp, Warning, TEXT("RequiredWaveToComplete %i"), RequiredWaveToComplete);

	if (WavesSurvived == 2)
	{
		//Plays the mission complete dialogue
		if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			if (GI->CurrentGameFlag < 2)
			{
				GI->StartDialogueRowName = "MissionComplete";
				GI->StartDialogue();
				GI->CurrentGameFlag++;
			}
		}
	}
	
	if (WavesSurvived >= RequiredWaveToComplete)
	{
		//UE_LOG(LogTemp, Warning, TEXT("RequiredWaveToComplete %i"), RequiredWaveToComplete);
		CompleteMission();

		//Plays the mission complete dialogue
		if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			GI->StartDialogueRowName = "MissionComplete";
			GI->StartDialogue();

			if (GI->CurrentGameFlag < 2)
			{
				GI->CurrentGameFlag = 3;
			}
			if (GI->CurrentGameFlag == 3 && UGameplayStatics::GetCurrentLevelName(GetWorld(),true) == TEXT("V2"))
			{
				GI->CurrentGameFlag++;
			}
			
			if (GI->CurrentGameFlag == 4 && UGameplayStatics::GetCurrentLevelName(GetWorld(),true) == TEXT("MetroV3"))
			{
				GI->CurrentGameFlag++;
			}
		}
	
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
}

void UMissionSubsystem::TryUnlockLevel() const
{
	if (!bIsCompleted) return;

	// Level unlock 
	UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	//Sets the game flag based on where you are in the "story"
	if (GI->CurrentGameFlag == 3)
		GI->CurrentGameFlag++;
	
	if (GI->CurrentGameFlag == 2)
		GI->CurrentGameFlag++;
	
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
	
	//Plays the mission complete dialogue for return if mission is complete
	GI->StartDialogueRowName = "ReturnMissionComplete";
}

void UMissionSubsystem::NewMission()
{
	bIsCompleted = false;
	WavesSurvived = 0;
}

void UMissionSubsystem::SetRequiredWavesToComplete(int32 NewRequired)
{
	RequiredWaveToComplete = NewRequired;
	//UE_LOG(LogTemp, Warning, TEXT("SetRequiredWavesToComplete %i"), RequiredWaveToComplete);
}


FText UMissionSubsystem::GetMissionStatusText() const
{
	if (bIsCompleted)
	{
		return FText::FromString(TEXT("Mission Complete!"));
	}

	return FText::Format(FText::FromString("Survive until Wave {0}"), FText::AsNumber(RequiredWaveToComplete));
}

