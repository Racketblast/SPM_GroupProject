// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChallengeSubsystem.generated.h"

UENUM(BlueprintType)
enum class EChallengeType : uint8
{
	None UMETA(DisplayName = "None"),
	NoJump UMETA(DisplayName = "Don't Jump"),
	PistolOnly UMETA(DisplayName = "Only Use Pistol"),
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

	void AssignNewChallenge();
	void CompleteCurrentChallenge();
	bool IsChallengeCompleted() const;

	FText GetChallengeDescription() const;

private:
	UPROPERTY()
	FChallengeData CurrentChallenge;

	TArray<FChallengeData> PossibleChallenges;
};

