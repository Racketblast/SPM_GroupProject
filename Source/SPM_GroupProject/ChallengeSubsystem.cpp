// Fill out your copyright notice in the Description page of Project Settings.


#include "ChallengeSubsystem.h"
#include "Kismet/KismetMathLibrary.h"

void UChallengeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Populate possible challenges
	PossibleChallenges.Add({ EChallengeType::NoJump, FText::FromString("Don't jump this wave."), false });
	PossibleChallenges.Add({ EChallengeType::PistolOnly, FText::FromString("Only use the pistol this wave."), false });

	AssignNewChallenge();
}

void UChallengeSubsystem::AssignNewChallenge()
{
	if (PossibleChallenges.Num() == 0) return;

	int32 Index = UKismetMathLibrary::RandomInteger(PossibleChallenges.Num());
	CurrentChallenge = PossibleChallenges[Index];
	CurrentChallenge.bIsCompleted = false;
	UE_LOG(LogTemp, Warning, TEXT("NewChallenge"));
}

void UChallengeSubsystem::CompleteCurrentChallenge()
{
	CurrentChallenge.bIsCompleted = true;
}

bool UChallengeSubsystem::IsChallengeCompleted() const
{
	return CurrentChallenge.bIsCompleted;
}

FText UChallengeSubsystem::GetChallengeDescription() const
{
	return CurrentChallenge.Description;
}


