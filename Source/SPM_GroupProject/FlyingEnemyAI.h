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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FlySpeed = 1.0f;

	// Väljer om fienden teleporterar till spelaren eller bara backar när den är fast
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	bool bTeleportIfStuck = true;

	// Väljer om fienden teleporterar till spelaren efter att MaxTimeToReachDestination timern har gott ut
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	bool bTeleportAfterTimer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	float MaxTimeToReachDestination = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float PostTeleportFireDelay = 2.0f;

	bool CanShoot() const; 
	void NotifyTeleported();

	void SetNewMoveTarget(const FVector& NewTarget);

	UFUNCTION()
	void TeleportToValidLocationNearPlayer();

	bool IsFireCooldownElapsed() const; 
	void NotifyFired();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FireCooldown = 2.0f; // Sekunder mellan skot som fienden skjuter

	void IsMoving();

protected:
	virtual void BeginPlay() override;
	
	float ShootingRange = 600.f;
	
	bool bHasRecentlyShot = false;

	FVector CurrentTargetLocation;
	FVector LastLocation;
	float StuckCheckTimer = 0.0f;
	bool bIsDescendingStuck = false;
	
	float MaxAltitude = 1000.0f; 	// ändras i wave manager
	
	float MinAltitude = 100.f; 	// ändras i wave manager

	float DestinationStartTime = 0.f;

	bool bIsMovingToTarget;

	float LastFireTime = -9999.f;

private:
	float LastTeleportTime = -100.f;
};
