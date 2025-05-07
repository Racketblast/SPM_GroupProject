// Fill out your copyright notice in the Description page of Project Settings.


#include "ChallengeSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerGameInstance.h"
#include "EngineUtils.h" 
#include "MissionAndChallengeManager.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UChallengeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("ChallengeSubsystem initialized, waiting for manager to load challenge data."));
	
	//AssignNewChallenge();
}

void UChallengeSubsystem::PreviewNextChallenge()
{
	if (PossibleChallenges.Num() == 0) return;
	
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
	bIsChallengeActive = false;
	
	UE_LOG(LogTemp, Warning, TEXT("New Challenge: %s"), *CurrentChallenge.Description.ToString());
}

void UChallengeSubsystem::ActivateCurrentChallenge()
{
	bIsChallengeActive = true; // väldigt viktig, startar faktiskt logiken för challenges.
	
	JustStartedChallenge = true; // för animation
	UE_LOG(LogTemp, Warning, TEXT("JustStartedChallenge: %s"), JustStartedChallenge ? TEXT("true") : TEXT("false"));
	
	GetWorld()->GetTimerManager().SetTimer( // För att aktivera en animation och sedan stänga av den vid rätt tillfälle 
	ResetChallengeStatusTimerHandle,
	this,
	&UChallengeSubsystem::ResetJustStartedChallenge,
	StartedChallengeAnimationTimer,  
	false  
	);
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

FText UChallengeSubsystem::GetChallengeDescription() const // Används för widget
{
	return CurrentChallenge.Description;
}

bool UChallengeSubsystem::GetJustStartedChallenge() const // Används för widget
{
	return JustStartedChallenge;
}

void UChallengeSubsystem::ResetJustStartedChallenge() // Används för widget
{
	JustStartedChallenge = false;
	UE_LOG(LogTemp, Warning, TEXT("JustStartedChallenge: %s"), JustStartedChallenge ? TEXT("true") : TEXT("false"));
}

bool UChallengeSubsystem::GetChallengeJustFailed() const  // Används för widget
{
	return bChallengeJustFailed;
}

bool UChallengeSubsystem::GetChallengeJustCompleted() const // Används för widget
{
	return bChallengeJustCompleted;
}

void UChallengeSubsystem::HandleChallengeSuccess()
{
	bChallengeJustCompleted = true;
	GiveChallengeReward();

	UE_LOG(LogTemp, Log, TEXT("Challenge Completed Successfully!"));
	//ResetChallengeStatus(); // För att bara aktivera en animation för en sekund och sedan sätta tillbaka variablerna till false.

	GetWorld()->GetTimerManager().SetTimer( // För att aktivera en animation och sedan stänga av den vid rätt tillfälle 
	ResetChallengeStatusTimerHandle,
	this,
	&UChallengeSubsystem::ResetChallengeStatus,
	SuccessAnimationTimer,  
	false  
	);
}

void UChallengeSubsystem::HandleChallengeFailure()
{
	bHasFailedCurrentChallenge = true;
	bChallengeJustFailed = true;

	UE_LOG(LogTemp, Warning, TEXT("Challenge Failed!"));
	//ResetChallengeStatus(); // För att bara aktivera en animation för en sekund och sedan sätta tillbaka variablerna till false.

	GetWorld()->GetTimerManager().SetTimer( // För att aktivera en animation och sedan stänga av den vid rätt tillfälle 
	ResetChallengeStatusTimerHandle,
	this,
	&UChallengeSubsystem::ResetChallengeStatus,
	FailedAnimationTimer,  
	false  
	);
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

// för animationer
void UChallengeSubsystem::SetAnimationTimers(float Success, float Failed, float StartedChallenge)
{
	SuccessAnimationTimer = Success;
	FailedAnimationTimer = Failed;
	StartedChallengeAnimationTimer = StartedChallenge;
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
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
		{
			Player->PickedUpMoney += RewardAmount;
		}
		//GI->Money += RewardAmount;
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
				ChallengeRewardMap.Empty(); // Rensar bort gammal data
				PossibleChallenges.Empty(); // Rensar bort gammal data

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

// För tids baserad challenge
void UChallengeSubsystem::StartWaveChallenge()
{
	//if (!bIsChallengeActive) return;              Kanske kan behövas, skulle antagligen fixa ifall timern startar med mindre tid en vad den ska ha
	
	if (CurrentChallenge.Type != EChallengeType::ClearWaveInTime)
		return;
	
	bHasFailedCurrentChallenge = false;
	bIsChallengeActive = true;
	
	if (CurrentChallenge.Type == EChallengeType::ClearWaveInTime)
	{
		if (UWorld* World = GetWorld())
		{
			// Få time limiten från manager
			for (TActorIterator<AMissionAndChallengeManager> It(World); It; ++It)
			{
				AMissionAndChallengeManager* Manager = *It;
				if (Manager)
				{
					CurrentWaveTimeLimit = Manager->ClearWaveTimeLimit;
					break;
				}
			}

			// Starta timeren
			World->GetTimerManager().SetTimer(
				TimerHandle_WaveTimeLimit,
				this,
				&UChallengeSubsystem::HandleWaveTimeExpired,
				CurrentWaveTimeLimit,
				false
			);

			UE_LOG(LogTemp, Warning, TEXT("Time-based challenge started! Time limit: %.1f seconds"), CurrentWaveTimeLimit);
		}
	}
}

void UChallengeSubsystem::HandleWaveTimeExpired()
{
	if (bIsChallengeActive && !bHasFailedCurrentChallenge)
	{
		if (CurrentChallenge.Type == EChallengeType::ClearWaveInTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("Challenge Failed: Wave not cleared in time!"));
			HandleChallengeFailure();
		}
	}
}

void UChallengeSubsystem::NotifyWaveCleared()
{
	if (!bIsChallengeActive || bHasFailedCurrentChallenge)
		return;
	
	if (CurrentChallenge.Type == EChallengeType::ClearWaveInTime)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_WaveTimeLimit);
		HandleChallengeSuccess();
	}
}

// för widget
float UChallengeSubsystem::GetRemainingChallengeTime() const
{
	if (CurrentChallenge.Type == EChallengeType::ClearWaveInTime && bIsChallengeActive)
	{
		float TimeRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(TimerHandle_WaveTimeLimit);
		return FMath::Max(TimeRemaining, 0.f);
	}
	return -1.f; 
}

// för widget
bool UChallengeSubsystem::ShouldShowChallengeTimer() const
{
	return CurrentChallenge.Type == EChallengeType::ClearWaveInTime && bIsChallengeActive;
}

