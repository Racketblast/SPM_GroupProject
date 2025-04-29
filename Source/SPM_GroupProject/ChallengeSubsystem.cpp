// Fill out your copyright notice in the Description page of Project Settings.


#include "ChallengeSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerGameInstance.h"
#include "EngineUtils.h" 
#include "MissionAndChallengeManager.h"

void UChallengeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("ChallengeSubsystem initialized, waiting for manager to load challenge data."));
	
	//AssignNewChallenge();
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

void UChallengeSubsystem::SetRewardMoneyAmount(int32 MoneyAmount)
{
	RewardMoneyAmount = MoneyAmount;
}

void UChallengeSubsystem::GiveChallengeReward()
{
	int32* FoundReward = ChallengeRewardMap.Find(CurrentChallenge.Type);
	int32 RewardAmount = FoundReward ? *FoundReward : RewardMoneyAmount; // Default reward

	if (!FoundReward)
	{
		UE_LOG(LogTemp, Warning, TEXT("No custom reward found for challenge %s. Using default."), *UEnum::GetValueAsString(CurrentChallenge.Type));
	}

	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GI->Money += RewardAmount;
		UE_LOG(LogTemp, Warning, TEXT("Challenge reward: +%d money!"), RewardAmount);
	}
}

void UChallengeSubsystem::LoadChallengeDataFromManager()
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AMissionAndChallengeManager> It(World); It; ++It)
		{
			AMissionAndChallengeManager* Manager = *It;
			if (Manager)
			{
				ChallengeRewardMap.Empty(); // Clear old data
				PossibleChallenges.Empty(); // Clear old data

				for (const FChallengeRewardData& Entry : Manager->ChallengeRewards)
				{
					PossibleChallenges.Add({ Entry.ChallengeType, Entry.ChallengeDescription, false });
					ChallengeRewardMap.Add(Entry.ChallengeType, Entry.RewardAmount);
				}

				RewardMoneyAmount = Manager->DefaultChallengeRewardAmount;
				UE_LOG(LogTemp, Warning, TEXT("Challenge data loaded from manager."));
				UE_LOG(LogTemp, Warning, TEXT("Loaded %d challenges into the system."), PossibleChallenges.Num());
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No Challenge Manager found in this level."));
}

// Används i PlayerCharacter
void UChallengeSubsystem::NotifyPlayerJumped()
{
	if (!bIsChallengeActive) return; // För att man inte ska kunna faila challengen under grace period.
	
	if (CurrentChallenge.Type == EChallengeType::NoJump && !bHasFailedCurrentChallenge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Challenge Failed: Player jumped!"));
		HandleChallengeFailure();
	}
}

// Används i enemy AI blueprint
void UChallengeSubsystem::NotifyPlayerDamaged()
{
	if (!bIsChallengeActive) return; 

	if (CurrentChallenge.Type == EChallengeType::NoDamage && !bHasFailedCurrentChallenge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Challenge Failed: Player took damage!"));
		HandleChallengeFailure();
	}
}

// Används i PlayerCharacter
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