// Fill out your copyright notice in the Description page of Project Settings.


#include "CollectableBox.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACollectableBox::ACollectableBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PhysicsRoot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsRoot"));
	RootComponent = PhysicsRoot;

	CollectableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollectableMesh"));
	CollectableMesh->SetupAttachment(PhysicsRoot);
	
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
	
	StartVector = CollectableMesh->GetRelativeLocation();

	//Should be deleted when all guns uses the TakeDamage-function
	FVector HelpVector = GetActorLocation();
	HelpVector.Z += 10;
	SetActorLocation(HelpVector);
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
		CollectableMesh->SetRelativeLocation(StartVector);
		PlatformVelocity = -PlatformVelocity;
	}
	else
	{
		FVector LocalVector = CollectableMesh->GetRelativeLocation();
		LocalVector = LocalVector + PlatformVelocity * DeltaTime;
		CollectableMesh->SetRelativeLocation(LocalVector);
	}
}

void ACollectableBox::RotateBox(float const DeltaTime)
{
	CollectableMesh->AddLocalRotation(RotationVelocity*DeltaTime);
}

bool ACollectableBox::ShouldBoxReturn() const
{
	return GetDistanceMoved() > DistFloat;
}

float ACollectableBox::GetDistanceMoved() const
{
	return FVector::Dist(StartVector, CollectableMesh->GetRelativeLocation());
}

void ACollectableBox::CollectableBoxTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Picked Up Something"));
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