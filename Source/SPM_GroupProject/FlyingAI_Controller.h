// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI_Controller.h"
#include "FlyingAI_Controller.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API AFlyingAI_Controller : public AAI_Controller
{
	GENERATED_BODY()
public:
	//AFlyingAI_Controller();
	UPROPERTY(EditAnywhere, Category = "AI")
	float PlayerRangeThreshold = 600.f; // Distansen innan spelaren är "in range"

	bool HasLineOfSightToPlayer() const;

protected:
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnPossess(APawn* InPawn) override;
};
