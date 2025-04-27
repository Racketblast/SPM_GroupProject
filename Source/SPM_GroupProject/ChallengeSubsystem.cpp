// Fill out your copyright notice in the Description page of Project Settings.


#include "ChallengeSubsystem.h"
#include "Kismet/KismetMathLibrary.h"

void UChallengeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Lägg till olika challenges
	PossibleChallenges.Add({ EChallengeType::NoJump, FText::FromString("Don't jump this wave."), false });
	PossibleChallenges.Add({ EChallengeType::PistolOnly, FText::FromString("Only use the pistol this wave."), false });
	PossibleChallenges.Add({ EChallengeType::NoDamage, FText::FromString("Don't take any damage this wave."), false });

	AssignNewChallenge();
}

void UChallengeSubsystem::AssignNewChallenge()
{
	if (PossibleChallenges.Num() == 0) return;

	//int32 Index = UKismetMathLibrary::RandomInteger(PossibleChallenges.Num());
	int32 Index = -1;

	// Loopar tills vi får en challenge som vi inte hade på waven innan. 
	do 
	{
		Index = UKismetMathLibrary::RandomInteger(PossibleChallenges.Num());
	} 
	while (PossibleChallenges[Index].Type == LastChallengeType && PossibleChallenges.Num() > 1); 
	
	CurrentChallenge = PossibleChallenges[Index];
	CurrentChallenge.bIsCompleted = false;
	LastChallengeType = CurrentChallenge.Type; // För att undvika att spelaren får samma challenge på rad
	
	UE_LOG(LogTemp, Warning, TEXT("New Challenge: %s"), *CurrentChallenge.Description.ToString());
}

void UChallengeSubsystem::CompleteCurrentChallenge()
{
	CurrentChallenge.bIsCompleted = true;
	bHasFailedCurrentChallenge = false;
}

bool UChallengeSubsystem::IsChallengeCompleted() const
{
	return CurrentChallenge.bIsCompleted;
}

FText UChallengeSubsystem::GetChallengeDescription() const
{
	return CurrentChallenge.Description;
}


void UChallengeSubsystem::NotifyPlayerJumped()
{
	if (CurrentChallenge.Type == EChallengeType::NoJump && !bHasFailedCurrentChallenge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Challenge Failed: Player jumped!"));
		bHasFailedCurrentChallenge = true;
	}
}