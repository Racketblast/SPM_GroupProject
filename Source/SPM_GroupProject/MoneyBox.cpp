// Fill out your copyright notice in the Description page of Project Settings.


#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMoneyBox::AMoneyBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MoneyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoneyMesh"));
	
	MoneyTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	MoneyTriggerVolume->SetupAttachment(MoneyMesh);

	MoneyPickUpSound = CreateDefaultSubobject<USoundBase>(TEXT("PickUpSound"));
	
	if (MoneyTriggerVolume)
	{
		MoneyTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AMoneyBox::MoneyBoxTriggered);
	}
}

// Called when the game starts or when spawned
void AMoneyBox::BeginPlay()
{
	Super::BeginPlay();
	
	StartVector = GetActorLocation();
	
	if (bShouldDestroy)
	{
		GetWorldTimerManager().SetTimer(SelfDestructTimer, this, &AMoneyBox::SelfDestruct, LifeTime, false);
	}
}

// Called every frame
void AMoneyBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveBox(DeltaTime);
	RotateBox(DeltaTime);
}

void AMoneyBox::MoneyBoxTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
			{
				GI->Money += MoneyAmount;
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), MoneyPickUpSound, GetActorLocation());
				Destroy();
			}
		}
	}
}

void AMoneyBox::MoveBox(float DeltaTime)
{
	if(ShouldBoxReturn())
	{
		const FVector MoveDirection = PlatformVelocity.GetSafeNormal();
		StartVector = StartVector + MoveDirection * GetDistanceMoved();
		SetActorLocation(StartVector);
		PlatformVelocity = -PlatformVelocity;
	}
	else
	{
		FVector LocalVector = GetActorLocation();
		LocalVector = LocalVector + PlatformVelocity * DeltaTime;
		SetActorLocation(LocalVector);
	}
}

void AMoneyBox::RotateBox(float const DeltaTime)
{
	AddActorLocalRotation(RotationVelocity*DeltaTime);
}

bool AMoneyBox::ShouldBoxReturn() const
{
	return GetDistanceMoved() > DistFloat;
}

float AMoneyBox::GetDistanceMoved() const
{
	return FVector::Dist(StartVector, GetActorLocation());
}

void AMoneyBox::SelfDestruct()
{
	Destroy();
}