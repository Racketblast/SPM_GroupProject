// Fill out your copyright notice in the Description page of Project Settings.


#include "CollectableBox.h"
#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACollectableBox::ACollectableBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollectableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoneyMesh"));
	
	CollectableTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	CollectableTriggerVolume->SetupAttachment(CollectableMesh);

	CollectablePickUpSound = CreateDefaultSubobject<USoundBase>(TEXT("PickUpSound"));
	
	if (CollectableTriggerVolume)
	{
		CollectableTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ACollectableBox::CollectableBoxTriggered);
	}

}

// Called when the game starts or when spawned
void ACollectableBox::BeginPlay()
{
	Super::BeginPlay();
	
	StartVector = GetActorLocation();
	
}

// Called every frame
void ACollectableBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	MoveBox(DeltaTime);
	RotateBox(DeltaTime);
}

void ACollectableBox::MoveBox(float DeltaTime)
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

void ACollectableBox::RotateBox(float const DeltaTime)
{
	AddActorLocalRotation(RotationVelocity*DeltaTime);
}

bool ACollectableBox::ShouldBoxReturn() const
{
	return GetDistanceMoved() > DistFloat;
}

float ACollectableBox::GetDistanceMoved() const
{
	return FVector::Dist(StartVector, GetActorLocation());
}

void ACollectableBox::CollectableBoxTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Happen"));
	CollectableBoxTriggeredFunction(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void ACollectableBox::CollectableBoxTriggeredFunction(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			UE_LOG(LogTemp, Warning, TEXT("CollectableBoxTriggered"));
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollectablePickUpSound, GetActorLocation());
			Destroy();
		}
	}
}