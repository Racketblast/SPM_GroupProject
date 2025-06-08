// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionAndChallengeManager.h"
#include "MissionSubsystem.h"
#include "ChallengeSubsystem.h"
#include "Engine/World.h"

AMissionAndChallengeManager::AMissionAndChallengeManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMissionAndChallengeManager::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LogTemp, Warning, TEXT("SetRequiredWavesToComplete BeginPlay %i"), RequiredWavesToComplete);
	
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
			ChallengeSubsystem->LoadChallengeDataFromManager();
			
			ChallengeSubsystem->SetRewardMoneyAmount(DefaultChallengeRewardAmount);

			ChallengeSubsystem->SetAnimationTimers(SuccessAnimationTimer, FailedAnimationTimer, StartedChallengeAnimationTimer); // för animationer

			ChallengeSubsystem->SetTimeToAdd(TimeToAddOnEnemyKilled);
		}
	}
}

