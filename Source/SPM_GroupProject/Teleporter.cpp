// Fill out your copyright notice in the Description page of Project Settings.


#include "Teleporter.h"

#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "PlayerGameInstance.h"
#include "PlayerCharacter.h"
#include "MissionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

// Sets default values
ATeleporter::ATeleporter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CubeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMeshComponent"));
	CubeMeshComponent->SetupAttachment(GetRootComponent());

	TeleportSkyBeam = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TeleportSkyBeam"));
	TeleportSkyBeam->SetupAttachment(CubeMeshComponent);
	TeleportSkyBeam->bAutoActivate = false;
}

void ATeleporter::Use_Implementation(APlayerCharacter* Player)
{
		if (!CachedGameInstance)
		return;
		// If you can teleport
		// Checks if in wave and if you have level unlocked
		if (!CachedGameInstance->bIsWave && CachedGameInstance->UnlockedLevels.Contains(TargetLevelName))
		{
			CachedGameInstance->Money += Player->PickedUpMoney;
			if (TeleportSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportSound, GetActorLocation());
			}
			// För level unlock 
			if (UMissionSubsystem* MissionSub = CachedGameInstance->GetSubsystem<UMissionSubsystem>())
			{
				MissionSub->TryUnlockLevel();
			}
			Teleport();
		}
		else
		{
			if (CantTeleportSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantTeleportSound, GetActorLocation());
			}
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
	//if it is a wave or if player does not have access to the level
	if (bOldWaveValue || !bHasAccess)
	{
		if (WaveMaterial)
		{
			CubeMeshComponent->SetMaterial(0, WaveMaterial);
		}
		if (TeleportSkyBeam && UGameplayStatics::GetCurrentLevelName(this,true) != TEXT("Hub"))
		{
			TeleportSkyBeam->Deactivate(); // Turn off effect if access denied
		}
	}
	else
	{
		if (GracePeriodMaterial)
		{
			CubeMeshComponent->SetMaterial(0, GracePeriodMaterial);
		}
		if (TeleportSkyBeam && UGameplayStatics::GetCurrentLevelName(this,true) != TEXT("Hub"))
		{
			TeleportSkyBeam->Activate(); // Turn on effect if access granted
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