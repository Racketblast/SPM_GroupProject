// Fill out your copyright notice in the Description page of Project Settings.


#include "ChallengeSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerGameInstance.h"

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

	bHasFailedCurrentChallenge = false;
	bIsChallengeActive = true;
	
	UE_LOG(LogTemp, Warning, TEXT("New Challenge: %s"), *CurrentChallenge.Description.ToString());
}

void UChallengeSubsystem::CompleteCurrentChallenge()
{
	if (!bIsChallengeActive) return;

	if (!bHasFailedCurrentChallenge)
	{
		HandleChallengeSuccess();
	}
	else
	{
		HandleChallengeFailure();
	}

	CurrentChallenge.bIsCompleted = true;
	bIsChallengeActive = false;
}

FText UChallengeSubsystem::GetChallengeDescription() const
{
	return CurrentChallenge.Description;
}

bool UChallengeSubsystem::GetChallengeJustFailed() const
{
	return bChallengeJustFailed;
}

bool UChallengeSubsystem::GetChallengeJustCompleted() const
{
	return bChallengeJustCompleted;
}

void UChallengeSubsystem::HandleChallengeSuccess()
{
	bChallengeJustCompleted = true;
	GiveChallengeReward();

	UE_LOG(LogTemp, Log, TEXT("Challenge Completed Successfully!"));
}

void UChallengeSubsystem::HandleChallengeFailure()
{
	bHasFailedCurrentChallenge = true;
	bChallengeJustFailed = true;

	UE_LOG(LogTemp, Warning, TEXT("Challenge Failed!"));
}

void UChallengeSubsystem::ResetChallengeStatus()
{
	bChallengeJustCompleted = false;
	bChallengeJustFailed = false;
}

void UChallengeSubsystem::GiveChallengeReward()
{
	EChallengeRewardType RewardType = EChallengeRewardType::Money; 

	switch (RewardType)
	{
	case EChallengeRewardType::Money:
		{
			int32 MoneyAmount = 100; // Kan ändra mängden senare
			if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
			{
				GI->Money += MoneyAmount;
				UE_LOG(LogTemp, Warning, TEXT("Challenge reward: +%d money!"), MoneyAmount);
			}
			break;
		}
		// lägg till fler rewards senare

	default:
		break;
	}
}

void UChallengeSubsystem::NotifyPlayerJumped()
{
	if (!bIsChallengeActive) return; // För att man inte ska kunna faila challengen under grace period.
	
	if (CurrentChallenge.Type == EChallengeType::NoJump && !bHasFailedCurrentChallenge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Challenge Failed: Player jumped!"));
		HandleChallengeFailure();
	}
}

// Används ännu inte, måste kalla denna funktion från vart spelaren tar skada. 
void UChallengeSubsystem::NotifyPlayerDamaged()
{
	if (!bIsChallengeActive) return; // Ignore if we're not in an active wave.

	if (CurrentChallenge.Type == EChallengeType::NoDamage && !bHasFailedCurrentChallenge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Challenge Failed: Player took damage!"));
		HandleChallengeFailure();
	}
}

// Används ännu inte. 
void UChallengeSubsystem::NotifyWeaponFired(FName WeaponName)
{
	if (!bIsChallengeActive) return;
    
	if (CurrentChallenge.Type == EChallengeType::PistolOnly && !bHasFailedCurrentChallenge)
	{
		if (WeaponName != "Pistol") 
		{
			UE_LOG(LogTemp, Warning, TEXT("Challenge Failed: Fired with wrong weapon: %s"), *WeaponName.ToString());
			HandleChallengeFailure();
		}
	}
}