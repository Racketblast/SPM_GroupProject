// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChallengeSubsystem.generated.h"

UENUM(BlueprintType)
enum class EChallengeRewardType : uint8
{
	None UMETA(DisplayName = "None"),
	Money UMETA(DisplayName = "Money"),
	// Lägg till fler reward typer senare, som ammo 
};

UENUM(BlueprintType)
enum class EChallengeType : uint8
{
	None UMETA(DisplayName = "None"),
	NoJump UMETA(DisplayName = "Don't Jump"),
	PistolOnly UMETA(DisplayName = "Only Use Pistol"),
	NoDamage UMETA(DisplayName = "Don't Take Any Damage"),
	ClearWaveInTime UMETA(DisplayName = "Clear the wave within time"),
	// Lägg till flera challenges här
};

USTRUCT(BlueprintType)
struct FChallengeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	EChallengeType Type = EChallengeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	bool bIsCompleted = false;
};

UCLASS()
class SPM_GROUPPROJECT_API UChallengeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void PreviewNextChallenge();

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void ActivateCurrentChallenge();
	
	void CompleteCurrentChallenge();

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void NotifyPlayerJumped();

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void NotifyPlayerDamaged();

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void NotifyWeaponFired(FName WeaponName);

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	FText GetChallengeDescription() const;
	
	UPROPERTY(BlueprintReadOnly, Category = "Challenge")
	bool bChallengeJustFailed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Challenge")
	bool bChallengeJustCompleted = false;
	
	void HandleChallengeSuccess();
	
	void HandleChallengeFailure();

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	bool GetChallengeJustFailed() const;

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	bool GetChallengeJustCompleted() const;

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void ResetChallengeStatus();

	void GiveChallengeReward();

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void SetRewardMoneyAmount(int32 MoneyAmount);

	void SetTimeToAdd(float NewTimeToAdd);

	void LoadChallengeDataFromManager();

	// för animationer
	FTimerHandle ResetChallengeStatusTimerHandle; // för animationer
	
	float SuccessAnimationTimer = 1;
	float FailedAnimationTimer = 1;

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	void SetAnimationTimers(float Success, float Failed, float StartedChallenge);

	bool JustStartedChallenge = false;
	float StartedChallengeAnimationTimer = 1;
	
	UFUNCTION(BlueprintCallable, Category = "Challenge")
	bool GetJustStartedChallenge() const; 
	void ResetJustStartedChallenge();

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	int32 GetCurrentChallengeRewardAmount() const; 
	
	// För tids baserad challenge
	FTimerHandle TimerHandle_WaveTimeLimit;
	FTimerHandle ResetJustStartedChallengeTimerHandle;
	float CurrentWaveTimeLimit = 0.0f;

	void StartWaveChallenge();
	void HandleWaveTimeExpired();
	void NotifyWaveCleared();

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	float GetRemainingChallengeTime() const;

	UFUNCTION(BlueprintCallable, Category = "Challenge")
	bool ShouldShowChallengeTimer() const;

	UFUNCTION(BlueprintCallable, Category = "Challenges") //  Kalla på denna funktion när fienderna dör
	void AddTimeToWaveChallenge(); 

private:
	UPROPERTY()
	FChallengeData CurrentChallenge;

	TArray<FChallengeData> PossibleChallenges;
	
	bool bHasFailedCurrentChallenge = false;

	EChallengeType LastChallengeType = EChallengeType::None;

	bool bIsChallengeActive = true;

	float TimeToAdd = 1.0f;

	UPROPERTY()
	int32 RewardMoneyAmount = 100;

	TMap<EChallengeType, int32> ChallengeRewardMap;
};

