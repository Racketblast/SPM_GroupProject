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

void AVendingMachine::ShowInteractable_Implementation(bool bShow)
{
	StaticMeshComponent->SetRenderCustomDepth(bShow);
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
					if (Weapon->TotalAmmo < Weapon->MaxTotalAmmo)
					{
						if (IfPlayerHasEnoughMoney(Player,GI))
						{
							Weapon->TotalAmmo += Weapon->MaxTotalAmmo/5;
							if (Weapon->TotalAmmo > Weapon->MaxTotalAmmo)
							{
								Weapon->TotalAmmo = Weapon->MaxTotalAmmo;
							}
						}
					}
					else
					{
						if (CantBuySound)
						{
							UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, GetActorLocation());
						}
					}
				}
				break;
			case EVendingMachineSpewOut::AmmoShotgun:
				if (AGun* Weapon = Player->GetWeaponInstance("Shotgun"))
				{
					if (Weapon->TotalAmmo < Weapon->MaxTotalAmmo)
					{
						if (IfPlayerHasEnoughMoney(Player,GI))
						{
							Weapon->TotalAmmo += Weapon->MaxTotalAmmo/5;
							if (Weapon->TotalAmmo > Weapon->MaxTotalAmmo)
							{
								Weapon->TotalAmmo = Weapon->MaxTotalAmmo;
							}
						}
					}
					else
					{
						if (CantBuySound)
						{
							UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, GetActorLocation());
						}
					}
				}
				break;
			case EVendingMachineSpewOut::AmmoRocketLauncher:
				if (AGun* Weapon = Player->GetWeaponInstance("RocketLauncher"))
				{
					if (Weapon->TotalAmmo < Weapon->MaxTotalAmmo)
					{
						if (IfPlayerHasEnoughMoney(Player,GI))
						{
							Weapon->TotalAmmo += Weapon->MaxTotalAmmo/5;
							if (Weapon->TotalAmmo > Weapon->MaxTotalAmmo)
							{
								Weapon->TotalAmmo = Weapon->MaxTotalAmmo;
							}
						}
					}
					else
					{
						if (CantBuySound)
						{
							UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, GetActorLocation());
						}
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
				if (Player->PickedUpMoney >= SpewOutCost)
				{
					Player->PickedUpMoney -= SpewOutCost;
				}
				else
				{
					SpewOutCost -= Player->PickedUpMoney;
					GI->Money -= SpewOutCost;
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

