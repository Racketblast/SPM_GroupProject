// Fill out your copyright notice in the Description page of Project Settings.


#include "ArenaGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

void AArenaGameMode::FadeIn(const AActor* PlayingActor)
{
	if (TeleportInSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportInSound, PlayingActor->GetActorLocation());
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
	if (TeleportOutSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportOutSound, PlayingActor->GetActorLocation());
	}
	if (FadeOutTransition)
	{
		FMovieSceneSequencePlaybackSettings Settings;
		ALevelSequenceActor* OutActor;
		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), FadeOutTransition, Settings, OutActor);
		SequencePlayer->Play();
	}
}

void AArenaGameMode::PlayerDeath()
{
	UGameplayStatics::OpenLevel(this, "Hub");
}