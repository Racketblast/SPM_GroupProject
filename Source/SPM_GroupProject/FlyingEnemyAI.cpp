// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingEnemyAI.h"
#include "FlyingAI_Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

AFlyingEnemyAI::AFlyingEnemyAI()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AFlyingAI_Controller::StaticClass();

}

void AFlyingEnemyAI::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
}

void AFlyingEnemyAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR
	DrawFlyableZRange();
#endif
}

void AFlyingEnemyAI::SetMaxAltitude(float Altitude)
{
	MaxAltitude = Altitude;
}

float AFlyingEnemyAI::GetMaxAltitude() const
{
	return MaxAltitude;
}

void AFlyingEnemyAI::SetMinAltitude(float Altitude)
{
	MinAltitude = Altitude;
}

float AFlyingEnemyAI::GetMinAltitude() const
{
	return MinAltitude;
}

void AFlyingEnemyAI::DrawFlyableZRange()
{
	FVector ActorLocation = GetActorLocation();

	// Debug sphere för min altitude 
	FVector MinZLocation = FVector(ActorLocation.X, ActorLocation.Y, MinAltitude);
	DrawDebugSphere(GetWorld(), MinZLocation, 40.f, 12, FColor::Red, false, -1.f, 0, 2.f);

	// Debug sphere för max altitude
	FVector MaxZLocation = FVector(ActorLocation.X, ActorLocation.Y, MaxAltitude);
	DrawDebugSphere(GetWorld(), MaxZLocation, 40.f, 12, FColor::Green, false, -1.f, 0, 2.f);

	// Debug linje som är mellan dem två debug sphere 
	DrawDebugLine(GetWorld(), MinZLocation, MaxZLocation, FColor::Yellow, false, -1.f, 0, 1.f);
}