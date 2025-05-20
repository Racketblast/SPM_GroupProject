// Fill out your copyright notice in the Description page of Project Settings.


#include "ArenaGameMode.h"

#include "DialogueInfo.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "PlayerGameInstance.h"
#include "GameFramework/Character.h"

void AArenaGameMode::FadeIn(const AActor* PlayingActor)
{
	if (PlayingActor)
	{
		if (TeleportInSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportInSound, PlayingActor->GetActorLocation());
		}
	}
	if (FadeInTransition)
	{
		FMovieSceneSequencePlaybackSettings Settings;
		ALevelSequenceActor *OutActor;
		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), FadeInTransition, Settings, OutActor);
		SequencePlayer->Play();
	}
}

void AArenaGameMode::FadeOut(const AActor* PlayingActor)
{
	if (PlayingActor)
	{
		if (TeleportOutSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportOutSound, PlayingActor->GetActorLocation());
		}
	}
	if (FadeOutTransition)
	{
		FMovieSceneSequencePlaybackSettings Settings;
		ALevelSequenceActor* OutActor;
		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), FadeOutTransition, Settings, OutActor);
		SequencePlayer->Play();

		ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		Player->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	}
}

void AArenaGameMode::PlayerDeath()
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		FName DeathDialogue = "SoldierDied1";
		float TimeUntilDone = 0.f;
		if (FDialogueInfo* Row = GI->EventDialogueInfo->FindRow<FDialogueInfo>(DeathDialogue, TEXT("")))
		{
			if (GI->bCanPlayDialogue)
			{
				TimeUntilDone = Row->DialogueSound->Duration;
			}
		}
		GI->StartDialogueRowName = DeathDialogue;
		GI->StartDialogue();
		GI->StartDialogueRowName = "ReturnDeath1";

		if (TimeUntilDone != 0.f)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AArenaGameMode::ReturnToHub, TimeUntilDone, false);
		}
		else if (SequencePlayer->IsPlaying())
		{
			SequencePlayer->OnFinished.AddDynamic(this, &AArenaGameMode::ReturnToHub);
		}
		else
		{
			ReturnToHub();
		}
	}
	else
	{
		ReturnToHub();
	}
}

void AArenaGameMode::BeginPlay()
{
	Super::BeginPlay();
	FadeIn(this);
	
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (UGameplayStatics::GetCurrentLevelName(GetWorld(), true) == "Hub")
		{
			if (GI->CurrentGameFlag < 1)
			{
				GI->StartDialogueRowName = "Intro";
			}
			UE_LOG(LogTemp, Warning, TEXT("Starting Dialogue %i"),GI->CurrentGameFlag);
			GI->StartDialogue();
		}
	}
}

void AArenaGameMode::ReturnToHub()
{
	UE_LOG(LogTemp, Display, TEXT("Return to Hub"));
	UGameplayStatics::OpenLevel(this, "Hub");
}
