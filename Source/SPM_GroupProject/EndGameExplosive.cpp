// Fill out your copyright notice in the Description page of Project Settings.


#include "EndGameExplosive.h"

#include "ArenaGameMode.h"
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
	
	if (SequencePlayer->IsPlaying())
	{
		if (AArenaGameMode* GM = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			SequencePlayer->OnFinished.AddDynamic(GM, &AArenaGameMode::GoToEnding);
		}
	}
	
	Destroy();
}
