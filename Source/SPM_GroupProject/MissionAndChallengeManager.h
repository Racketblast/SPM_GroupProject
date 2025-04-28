// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionAndChallengeManager.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AMissionAndChallengeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AMissionAndChallengeManager();

protected:
	virtual void BeginPlay() override;

public:	
	// Missions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 RequiredWavesToComplete = 3;

	//Challenges
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	int32 ChallengeRewardMoneyAmount = 100;
};
