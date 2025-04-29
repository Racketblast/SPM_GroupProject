// Fill out your copyright notice in the Description page of Project Settings.


#include "Teleporter.h"

#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "PlayerGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATeleporter::ATeleporter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMeshComponent"));
	CubeMeshComponent->SetupAttachment(GetRootComponent());
	if (TargetLevel)
	{
		TargetLevelName = FName(*TargetLevel->GetName());
	}
}

void ATeleporter::BeginPlay()
{
	Super::BeginPlay();
	
	CachedGameInstance = Cast<UPlayerGameInstance>(GetGameInstance());
	
	if (CachedGameInstance)
	{
		bOldWaveValue = CachedGameInstance->bIsWave;
	}
	else
	{
		bOldWaveValue = false;
	}
	
	ChangeTexture();
}

void ATeleporter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!CachedGameInstance)
		return;

	if (bOldWaveValue == CachedGameInstance->bIsWave)
		return;

	bOldWaveValue = CachedGameInstance->bIsWave;
	ChangeTexture();
}

void ATeleporter::ChangeTexture()
{
	bool bHasAccess = CachedGameInstance->UnlockedLevels.Contains(TargetLevelName);
	UE_LOG(LogTemp, Display, TEXT("Can Teleport to %s: %s"), *TargetLevelName.ToString(), bHasAccess ? TEXT("true") : TEXT("false"));
	//UE_LOG(LogTemp, Display, TEXT("Can Teleport: %s"), CachedGameInstance->TeleportKeyArray[TeleportKeyNumber] ? TEXT("true") : TEXT("false"));
	//if it is a wave or if player does not have a key
	//if (bOldWaveValue || !CachedGameInstance->TeleportKeyArray[TeleportKeyNumber])
	if (bOldWaveValue || !bHasAccess)
	{
		if (WaveMaterial)
		{
			UE_LOG(LogTemp, Display, TEXT("Who Cant Teleport: %s"), *GetName());
			CubeMeshComponent->SetMaterial(0, WaveMaterial);
		}
	}
	else
	{
		if (GracePeriodMaterial)
		{
			UE_LOG(LogTemp, Display, TEXT("Who Can Teleport: %s"), *GetName());
			CubeMeshComponent->SetMaterial(0, GracePeriodMaterial);
		}
	}
}

void ATeleporter::Teleport()
{
	if (FadeOutTransition)
	{
		FMovieSceneSequencePlaybackSettings Settings;
		ALevelSequenceActor* OutActor;
		ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), FadeOutTransition, Settings, OutActor);
		SequencePlayer->Play();

		if (SequencePlayer)
		{
			SequencePlayer->OnFinished.AddDynamic(this, &ATeleporter::ChangeLevel);
		}
	}
}

void ATeleporter::ChangeLevel()
{
	UGameplayStatics::OpenLevel(this, TargetLevelName);
}