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