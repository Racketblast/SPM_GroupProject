// Fill out your copyright notice in the Description page of Project Settings.

#include "MissionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "PlayerGameInstance.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/Actor.h"

// Körs när subsystemet skapas, registrerar OnWorldInitialized så den vet när en ny värld (level) är laddad.
void UMissionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMissionSubsystem::OnWorldInitialized);
	
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem initialized."));
}

// Körs när subsystemet förstörs, tar bort alla callbacks från FWorldDelegates
void UMissionSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	
	UE_LOG(LogTemp, Log, TEXT("MissionSubsystem deinitialized."));
}

// Körs varje gång en ny nivå laddas
void UMissionSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
	NewMission(); // Nollställer variabler, så att man kan göra ett nytt mission.
	UE_LOG(LogTemp, Log, TEXT("New mission started for the new level!"));
}

// Anropas när spelaren klarar av en wave från wave managern. 
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
			}
		}
	}

	// Anropar CompleteMission() om man överlevt tillräckligt många waves
	if (WavesSurvived >= RequiredWaveToComplete)
	{
		//UE_LOG(LogTemp, Warning, TEXT("RequiredWaveToComplete %i"), RequiredWaveToComplete);
		CompleteMission();

		//Plays the mission complete dialogue
		if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			GI->StartDialogueRowName = "MissionComplete";
			GI->StartDialogue();
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

// Kallas från ATeleporter, i funktionen som körs när spelaren går tillbaka till hubben. 
void UMissionSubsystem::TryUnlockLevel() const
{
	// Kollar om du klarat av missionet
	if (!bIsCompleted) return;

	// Level unlock 
	UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GI) return;

	// Hämtar nuvarande nivånamn och lägger till nuvarande nivå till UnlockedLevels
	FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
	GI->LastCompletedLevel = CurrentLevel;
	GI->UnlockedLevels.Add(CurrentLevel);

	// Tar nästa nivå i LevelOrder och lägger till den också
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

