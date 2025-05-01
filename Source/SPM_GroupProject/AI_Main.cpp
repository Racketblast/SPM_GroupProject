// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Main.h"

// Sets default values
AAI_Main::AAI_Main()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

UBehaviorTree* AAI_Main::GetBehaviorTree() const
{
	return BehaviorTree;
}

// Called when the game starts or when spawned
void AAI_Main::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAI_Main::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAI_Main::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

