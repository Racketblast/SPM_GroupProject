// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionAndChallengeManager.h"
#include "MissionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// Sets default values
AMissionAndChallengeManager::AMissionAndChallengeManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AMissionAndChallengeManager::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UMissionSubsystem* MissionSubsystem = World->GetGameInstance()->GetSubsystem<UMissionSubsystem>())
		{
			MissionSubsystem->SetRequiredWavesToComplete(RequiredWavesToComplete);
		}
	}
}

// Called every frame
void AMissionAndChallengeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

