// Fill out your copyright notice in the Description page of Project Settings.


#include "MoneyBox.h"
#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

// Called when the game starts or when spawned
void AMoneyBox::BeginPlay()
{
	Super::BeginPlay();
	
	if (bShouldDestroy)
	{
		GetWorldTimerManager().SetTimer(SelfDestructTimer, this, &AMoneyBox::SelfDestruct, LifeTime, false);
		GetWorldTimerManager().SetTimer(FadeTimer, this, &AMoneyBox::SetFadeMaterial, LifeTime/2, false);
	}
}

void AMoneyBox::CollectableBoxTriggeredFunction(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			FString MapName = GetWorld()->GetMapName();
			MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
			if (MapName == "Hub")
			{
				if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
				{
					GI->Money += MoneyAmount;
				}
			}
			else
			{
				Player->PickedUpMoney += MoneyAmount;
			}
			
			UE_LOG(LogTemp, Warning, TEXT("MoneyBoxTriggered"));
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollectablePickUpSound, GetActorLocation());
			Destroy();
		}
	}
}

void AMoneyBox::SelfDestruct()
{
	Destroy();
}

void AMoneyBox::SetFadeMaterial()
{
	if (FadeMaterial)
	{
		CollectableMesh->SetMaterial(0, FadeMaterial);
	}
}
