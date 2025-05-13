// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArenaGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API AArenaGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	void FadeIn(const AActor* PlayingActor);
	void FadeOut(const AActor* PlayingActor);
	UFUNCTION()
	void PlayerDeath();
	
	UPROPERTY()
	class ULevelSequencePlayer* SequencePlayer;
protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, Category="Transition")
	class ULevelSequence* FadeInTransition;
	UPROPERTY(EditDefaultsOnly, Category="Transition")
	class ULevelSequence* FadeOutTransition;
	UPROPERTY(EditDefaultsOnly, Category="Transition")
	class USoundBase* TeleportInSound;
	UPROPERTY(EditDefaultsOnly, Category="Transition")
	class USoundBase* TeleportOutSound;
private:
	FTimerHandle TimerHandle;
	
	UFUNCTION()
	void ReturnToHub();
};


