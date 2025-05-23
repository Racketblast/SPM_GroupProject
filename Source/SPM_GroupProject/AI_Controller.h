// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI_Main.h"
#include "Perception/AIPerceptionTypes.h"
#include "AI_Controller.generated.h"


/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API AAI_Controller : public AAIController
{
	GENERATED_BODY()

public:
	explicit AAI_Controller(FObjectInitializer const& FObjectInitializer);

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	class UAISenseConfig_Sight* SightConfig;

	void SetupPerceptionSystem();

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus);
};



