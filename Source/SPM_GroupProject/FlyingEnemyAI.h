// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI_Main.h"
#include "FlyingEnemyAI.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API AFlyingEnemyAI : public AAI_Main
{
	GENERATED_BODY()
public:
	AFlyingEnemyAI();

	void SetMaxAltitude(float Altitude); // Kallas från wave manager, i spawnd enemy funktionen

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UBehaviorTree* BehaviorTree;
	
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }*/

protected:
	virtual void BeginPlay() override;
	
	float ShootingRange = 600.f;
	
	bool bHasRecentlyShot = false;

	// ändras i wave manager
	float MaxAltitude = 1000.0f;
};
