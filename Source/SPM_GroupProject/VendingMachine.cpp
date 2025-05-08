// Fill out your copyright notice in the Description page of Project Settings.


#include "VendingMachine.h"

#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AVendingMachine::AVendingMachine()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
}
void AVendingMachine::Use_Implementation(APlayerCharacter* Player)
{
	UseVendingMachine();
}

void AVendingMachine::UseVendingMachine()
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(),0)))
		{
			switch (SpewOut)
			{
			case EVendingMachineSpewOut::Health:
				if (Player->PlayerHealth < Player->PlayerMaxHealth)
				{
					if (IfPlayerHasEnoughMoney(Player,GI))
					{
						Player->HealPlayer(SpewOutAmount);
					}
				}
				else
				{
					if (CantBuySound)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, GetActorLocation());
					}
				}
				break;
			case EVendingMachineSpewOut::AmmoRifle:
				if (AGun* Weapon = Player->GetWeaponInstance("Rifle"))
				{
					if (IfPlayerHasEnoughMoney(Player,GI))
					{
						Weapon->TotalAmmo += SpewOutAmount;
					}
				}
				break;
			default:
				break;
			}
		}
	}
	
}

bool AVendingMachine::IfPlayerHasEnoughMoney(APlayerCharacter* Player, UPlayerGameInstance* GI)
{
	if (Player)
	{
		if (GI)
		{
			if (Player->PickedUpMoney + GI->Money >= SpewOutCost)
			{
				if (Player->PickedUpMoney >= SpewOutAmount)
				{
					Player->PickedUpMoney -= SpewOutAmount;
				}
				else
				{
					SpewOutAmount -= Player->PickedUpMoney;
					GI->Money -= SpewOutAmount;
				}
				
				if (CanBuySound)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CanBuySound, GetActorLocation());
				}
				return true;
			}
		}
	}

	if (CantBuySound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, GetActorLocation());
	}
	return false;
}

