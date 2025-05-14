// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/KillZVolume.h"
#include "ArenaKillZVolume.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API AArenaKillZVolume : public AKillZVolume
{
	GENERATED_BODY()
	virtual void ActorEnteredVolume(class AActor* Other) override;
};
