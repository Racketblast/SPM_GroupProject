// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionAndChallengeManager.h"
#include "MissionSubsystem.h"
#include "ChallengeSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


AMissionAndChallengeManager::AMissionAndChallengeManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMissionAndChallengeManager::BeginPlay()
{
	Super::BeginPlay();

	// Missions
	if (UWorld* World = GetWorld())
	{
		if (UMissionSubsystem* MissionSubsystem = World->GetGameInstance()->GetSubsystem<UMissionSubsystem>())
		{
			MissionSubsystem->SetRequiredWavesToComplete(RequiredWavesToComplete);
		}
	}

	// Challenges
	if (UWorld* World = GetWorld())
	{
		if (UChallengeSubsystem* ChallengeSubsystem = World->GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
		{
			ChallengeSubsystem->LoadChallengeDataFromManager(); /*Jag skulle kunna lägga till en bool som skyddar från att detta blir kallat flera gånger,
																dock så skulle alla challenges då ha samma rewards på alla levels, då man nu utan boolen skulle kunna ändra hur mycket pengar man får från en challenge mellan levels*/
			ChallengeSubsystem->SetRewardMoneyAmount(DefaultChallengeRewardAmount);
		}
	}
}

