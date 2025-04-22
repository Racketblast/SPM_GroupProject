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

// Called when the game starts or when spawned
void AMoneyBox::BeginPlay()
{
	Super::BeginPlay();
	
	if (bShouldDestroy)
	{
		GetWorldTimerManager().SetTimer(SelfDestructTimer, this, &AMoneyBox::SelfDestruct, LifeTime, false);
	}
}

void AMoneyBox::CollectableBoxTriggeredFunction(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
			{
				GI->Money += MoneyAmount;
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollectablePickUpSound, GetActorLocation());
				Destroy();
			}
		}
	}
}

void AMoneyBox::SelfDestruct()
{
	Destroy();
}