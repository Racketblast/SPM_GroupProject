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

	void Tick(float DeltaTime);

	void DrawFlyableZRange();

	void SetMaxAltitude(float Altitude); // Kallas från wave manager, i spawnd enemy funktionen

	float GetMaxAltitude() const; // Används i FindPlayerLocation_Flying

	void SetMinAltitude(float Altitude); // Kallas från wave manager, i spawnd enemy funktionen

	float GetMinAltitude() const; // Används i FindPlayerLocation_Flying

	void TryLateralUnstick();

	void SetCurrentTargetLocation(const FVector& NewTarget);
	FVector GetCurrentTargetLocation() const;

protected:
	virtual void BeginPlay() override;
	
	float ShootingRange = 600.f;
	
	bool bHasRecentlyShot = false;

	FVector CurrentTargetLocation;
	FVector LastLocation;
	float StuckCheckTimer = 0.0f;
	bool bIsDescendingStuck = false;

	// ändras i wave manager
	float MaxAltitude = 1000.0f;

	// ändras i wave manager
	float MinAltitude = 100.f;  
};
