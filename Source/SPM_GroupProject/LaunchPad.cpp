// Fill out your copyright notice in the Description page of Project Settings.


#include "LaunchPad.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALaunchPad::ALaunchPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LaunchPadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaunchPadMesh"));
	
	LaunchPadTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	LaunchPadTriggerVolume->SetupAttachment(LaunchPadMesh);
	
	LaunchPadTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ALaunchPad::LaunchPadTriggered);
	//LaunchPadTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ALaunchPad::LaunchPadEndTrigger);
}

// Called when the game starts or when spawned
void ALaunchPad::BeginPlay()
{
	Super::BeginPlay();

	//Debug line
	/*FVector Start = GetActorLocation();
	FVector End = Start + LaunchPadMesh->GetComponentRotation().RotateVector(LaunchVector);
	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		FColor::Red,
		true,
		0,
		0,
		3.0f
	);*/
}

// Called every frame
void ALaunchPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALaunchPad::LaunchPadTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			FVector Velocity = LaunchPadMesh->GetComponentRotation().RotateVector(LaunchVector);
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), LaunchSound, GetActorLocation());
			Player->LaunchCharacter(Velocity, bOverrideXY, bOverrideZ);
			UE_LOG(LogTemp,Display,TEXT("LaunchPadTriggered"));
		}
	}
}

//Not used but could be useful
/*
void ALaunchPad::LaunchPadEndTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			
		}
	}
}
*/

