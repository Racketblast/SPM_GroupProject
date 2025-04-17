// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoBox.h"
#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAmmoBox::AAmmoBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoneyMesh"));
	
	AmmoTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	AmmoTriggerVolume->SetupAttachment(AmmoMesh);

	AmmoPickUpSound = CreateDefaultSubobject<USoundBase>(TEXT("PickUpSound"));
	
	if (AmmoTriggerVolume)
	{
		AmmoTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AAmmoBox::AmmoBoxTriggered);
	}
}

// Called when the game starts or when spawned
void AAmmoBox::BeginPlay()
{
	Super::BeginPlay();
	
	StartVector = GetActorLocation();
	
}

// Called every frame
void AAmmoBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	MoveBox(DeltaTime);
	RotateBox(DeltaTime);
}

void AAmmoBox::AmmoBoxTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			Player->ExtraMags += AmmoAmount;
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), AmmoPickUpSound, GetActorLocation());
			Destroy();
		}
	}
}

void AAmmoBox::MoveBox(float DeltaTime)
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

void AAmmoBox::RotateBox(float const DeltaTime)
{
	AddActorLocalRotation(RotationVelocity*DeltaTime);
}

bool AAmmoBox::ShouldBoxReturn() const
{
	return GetDistanceMoved() > DistFloat;
}

float AAmmoBox::GetDistanceMoved() const
{
	return FVector::Dist(StartVector, GetActorLocation());
}
