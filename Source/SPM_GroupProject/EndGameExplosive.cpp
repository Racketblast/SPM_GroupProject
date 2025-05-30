// Fill out your copyright notice in the Description page of Project Settings.


#include "EndGameExplosive.h"

#include "ArenaGameMode.h"
#include "DialogueInfo.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "PlayerGameInstance.h"

#include "Kismet/GameplayStatics.h"

void AEndGameExplosive::Explode()
{
    FVector ExplosionCenter = GetActorLocation();
	if (ExplosionEffectAsset)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffectAsset, ExplosionCenter, FRotator::ZeroRotator, true);
	}
	
	FMovieSceneSequencePlaybackSettings Settings;
	ALevelSequenceActor *OutActor;
	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), FadeOutTransition, Settings, OutActor);
	SequencePlayer->Play();

	float TimeUntilDone = 0.f;
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		FName EndDialogue = "WhatDidYouDo";
		GI->StartDialogueRowName = EndDialogue;
		GI->StartDialogue();
		if (FDialogueInfo* Row = GI->EventDialogueInfo->FindRow<FDialogueInfo>(EndDialogue, TEXT("")))
		{
			if (GI->bCanPlayDialogue)
			{
				TimeUntilDone = Row->DialogueSound->Duration;
			}
		}
	}

	
	
	if (TimeUntilDone != 0.f)
	{
		if (AArenaGameMode* GM = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, GM, &AArenaGameMode::GoToEnding, TimeUntilDone, false);
		}
	}
	else if (SequencePlayer->IsPlaying())
	{
		if (AArenaGameMode* GM = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			SequencePlayer->OnFinished.AddDynamic(GM, &AArenaGameMode::GoToEnding);
		}
	}
	
	Destroy();
}
