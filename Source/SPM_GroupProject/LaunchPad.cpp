// Fill out your copyright notice in the Description page of Project Settings.


#include "LaunchPad.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALaunchPad::ALaunchPad()
{
	LaunchPadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaunchPadMesh"));
	
	LaunchPadTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	LaunchPadTriggerVolume->SetupAttachment(LaunchPadMesh);
	
	LaunchPadTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ALaunchPad::LaunchPadTriggered);
}

void ALaunchPad::LaunchPadTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		//Checks if capsule touches and not anything else from the actor
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			FVector Velocity = LaunchPadMesh->GetComponentRotation().RotateVector(LaunchVector);
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), LaunchSound, GetActorLocation());
			Player->JumpCurrentCount = 1;
			Player->LaunchCharacter(Velocity, bOverrideXY, bOverrideZ);
			UE_LOG(LogTemp,Display,TEXT("LaunchPadTriggered"));
		}
	}
}
