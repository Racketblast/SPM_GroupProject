// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Controller.h"
#include "AI_Main.h"
#include "PlayerCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"


AAI_Controller::AAI_Controller(FObjectInitializer const& FObjectInitializer)
{
	SetupPerceptionSystem();
	
}

void AAI_Controller::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (AAI_Main* const AI = Cast<AAI_Main>(InPawn))
	{
		if (UBehaviorTree* const BehaviorTree = AI->GetBehaviorTree())
		{
			UBlackboardComponent* b;
			UseBlackboard(BehaviorTree->BlackboardAsset,  b);
			Blackboard = b;
			RunBehaviorTree(BehaviorTree);
			
		}
	}
}

void AAI_Controller::SetupPerceptionSystem()
{
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (SightConfig)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(
		TEXT("Perception Component")));
		SightConfig->SightRadius = 1500.0f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 25.f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(5.f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AAI_Controller::OnTargetDetected);
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}

	
}

void AAI_Controller::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	//Const?
	if (auto* const Player = Cast<APlayerCharacter>(Actor))
	{
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayerCharacter", Stimulus.WasSuccessfullySensed());
	}
}


