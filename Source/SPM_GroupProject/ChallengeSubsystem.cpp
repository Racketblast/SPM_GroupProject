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
	bHasFailedCurrentChallenge = false;
	if (PossibleChallenges.Num() == 0) return;
	
	int32 Index = -1;

	bool OnlyHasPistol = true;

	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (GI->HasBought("Rifle") || GI->HasBought("Shotgun") || GI->HasBought("Rocketlauncher"))
		{
			OnlyHasPistol = false;
		}
	}

	// Loopar tills vi får en challenge som vi inte hade på waven innan. 
	do 
	{
		Index = UKismetMathLibrary::RandomInteger(PossibleChallenges.Num());
	} 
	while (PossibleChallenges.Num() > 1 && (
		PossibleChallenges[Index].Type == LastChallengeType ||
		(OnlyHasPistol && PossibleChallenges[Index].Type == EChallengeType::PistolOnly)));
		
	//PossibleChallenges[Index].Type == LastChallengeType && PossibleChallenges.Num() > 1 && (!OnlyHasPistol && PossibleChallenges[Index].Type == EChallengeType::PistolOnly)
	
	CurrentChallenge = PossibleChallenges[Index];
	CurrentChallenge.bIsCompleted = false;
	LastChallengeType = CurrentChallenge.Type; // För att undvika att spelaren får samma challenge på rad
	
	bIsChallengeActive = false;
	
	UE_LOG(LogTemp, Warning, TEXT("New Challenge: %s"), *CurrentChallenge.Description.ToString());


	
	// för animationer, var orginellt i ActivateCurrentChallenge
	JustStartedChallenge = true; // för animatio
	
	UE_LOG(LogTemp, Warning, TEXT("JustStartedChallenge: %s"), JustStartedChallenge ? TEXT("true") : TEXT("false"));
	//UE_LOG(LogTemp, Warning, TEXT("StartedChallengeAnimationTimer: %f"), StartedChallengeAnimationTimer);
	/*if (StartedChallengeAnimationTimer <= 0)
	{
		StartedChallengeAnimationTimer = 4.f;
	}*/
	GetWorld()->GetTimerManager().SetTimer( // För att aktivera en animation och sedan stänga av den vid rätt tillfälle 
	ResetJustStartedChallengeTimerHandle,
	this,
	&UChallengeSubsystem::ResetJustStartedChallenge,
	StartedChallengeAnimationTimer,  
	false  
	);
	//UE_LOG(LogTemp, Warning, TEXT("StartedChallengeAnimationTimer: %f"), StartedChallengeAnimationTimer);
}

void UChallengeSubsystem::ActivateCurrentChallenge()
{
	bIsChallengeActive = true; // väldigt viktig, startar faktiskt logiken för challenges.
}

void UChallengeSubsystem::CompleteCurrentChallenge()
{
	if (!bIsChallengeActive) return;

	if (!bHasFailedCurrentChallenge)
	{
		HandleChallengeSuccess();
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
	//UE_LOG(LogTemp, Warning, TEXT("ResetJustStartedChallenge"));
	JustStartedChallenge = false;
	//UE_LOG(LogTemp, Warning, TEXT("Second JustStartedChallenge: %s"), JustStartedChallenge ? TEXT("true") : TEXT("false"));
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
	//UE_LOG(LogTemp, Warning, TEXT("Timers set: Success: %f, Failed: %f, Started: %f"), SuccessAnimationTimer, FailedAnimationTimer, StartedChallengeAnimationTimer);
}

void UChallengeSubsystem::GiveChallengeReward()
{
	/*int32* FoundReward = ChallengeRewardMap.Find(CurrentChallenge.Type);
	int32 RewardAmount = FoundReward ? *FoundReward : RewardMoneyAmount; */

	int32 RewardAmount = GetCurrentChallengeRewardAmount();

	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
		{
			Player->PickedUpMoney += RewardAmount;
		}
		UE_LOG(LogTemp, Warning, TEXT("Challenge reward: +%d money!"), RewardAmount);
	}
}

int32 UChallengeSubsystem::GetCurrentChallengeRewardAmount() const
{
	const int32* FoundReward = ChallengeRewardMap.Find(CurrentChallenge.Type);
	
	if (!FoundReward)
	{
		//UE_LOG(LogTemp, Warning, TEXT("No custom reward found for challenge %s. Using default."), *UEnum::GetValueAsString(CurrentChallenge.Type));
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("Reward found for challenge %s. Reward amount is %d."), *UEnum::GetValueAsString(CurrentChallenge.Type), FoundReward ? *FoundReward : RewardMoneyAmount);
	return FoundReward ? *FoundReward : RewardMoneyAmount;
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
				PreviewNextChallenge(); 
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

// Denna funktion används inte just nu, men om man kallar på den när fiender dör så kommer spelaren att få tillbaka lite tid till timern, när dem har challngen att klara en wave inom en viss tid. 
void UChallengeSubsystem::AddTimeToWaveChallenge()
{
	if (CurrentChallenge.Type != EChallengeType::ClearWaveInTime || !bIsChallengeActive)
		return;

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();

		// Hämtar den resterande tiden
		float TimeRemaining = TimerManager.GetTimerRemaining(TimerHandle_WaveTimeLimit);
		if (TimeRemaining <= 0.f)
			return;

		// lägger till tid
		float NewTime = TimeRemaining + TimeToAdd;

		// Rensar den nuvarande timern
		TimerManager.ClearTimer(TimerHandle_WaveTimeLimit);

		// Starta om timern med den nya tiden
		TimerManager.SetTimer(
			TimerHandle_WaveTimeLimit,
			this,
			&UChallengeSubsystem::HandleWaveTimeExpired,
			NewTime,
			false
		);

		UE_LOG(LogTemp, Log, TEXT("Added %.1f seconds to wave timer. New time: %.1f"), TimeToAdd, NewTime);
	}
}

void UChallengeSubsystem::SetTimeToAdd(float NewTimeToAdd)
{
	TimeToAdd = NewTimeToAdd;
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

