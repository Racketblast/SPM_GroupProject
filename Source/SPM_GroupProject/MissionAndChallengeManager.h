// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChallengeSubsystem.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionAndChallengeManager.generated.h"

USTRUCT(BlueprintType)
struct FChallengeRewardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EChallengeType ChallengeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChallengeDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardAmount = 100;
};

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
	int32 DefaultChallengeRewardAmount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	TArray<FChallengeRewardData> ChallengeRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	float ClearWaveTimeLimit = 30.0f;

	// för animationer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge") // ändrade från int32 till float, om detta skappar problem, så ändra tillbaka
	float SuccessAnimationTimer = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	float FailedAnimationTimer = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	float StartedChallengeAnimationTimer = 1;
};
