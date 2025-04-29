// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerGameInstance.h"

#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UPlayerGameInstance::HasBought(const EUpgradeType Upgrade) const
{
	return UpgradeArray.Contains(Upgrade);
}

bool UPlayerGameInstance::HasBought(const FName Upgrade) const
{
	for (EUpgradeType UpgradeType : UpgradeArray)
	{
		if (FName(*StaticEnum<EUpgradeType>()->GetNameStringByValue(static_cast<int64>(UpgradeType))) == Upgrade)
		{
			return true;
		}
	}
	return false;
}

int32 UPlayerGameInstance::GetUpgradeCost(const EUpgradeType Upgrade) const
{
	switch (Upgrade)
	{
	case EUpgradeType::Pistol:
		return 0;
	case EUpgradeType::Rifle:
		return 20;
	case EUpgradeType::Health20:
		return 100;
	case EUpgradeType::Speed20:
		return 100;
	case EUpgradeType::Jump50:
		return 200;
	default:
		return 0;
	}
}

EUpgradeCategory UPlayerGameInstance::GetUpgradeCategory(const EUpgradeType Upgrade) const
{
	switch (Upgrade)
	{
	case EUpgradeType::Pistol:
		return EUpgradeCategory::Weapon;
	case EUpgradeType::Rifle:
		return EUpgradeCategory::Weapon;
	case EUpgradeType::Health20:
		return EUpgradeCategory::PlayerStats;
	case EUpgradeType::Speed20:
		return EUpgradeCategory::PlayerStats;
	case EUpgradeType::Jump50:
		return EUpgradeCategory::PlayerStats;
	default:
		return EUpgradeCategory::None;
	}
}

void UPlayerGameInstance::BuyUpgrade(const EUpgradeType Upgrade, USoundBase* CanBuySound, USoundBase* CantBuySound)
{
	// If you don't own the product
	if (!HasBought(Upgrade))
	{
		// If you can buy the product
		if (GetUpgradeCost(Upgrade) <= Money)
		{
			if (CanBuySound)
			{
				if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CanBuySound, Player->GetActorLocation());
				}
			}
			Money -= GetUpgradeCost(Upgrade);
			UpgradeArray.Add(Upgrade);
			
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				if (GetUpgradeCategory(Upgrade) == EUpgradeCategory::Weapon)
				{
					SetCurrentWeapon(Upgrade);
					Player->SelectWeapon(*ConvertUpgradeTypeToString(Upgrade));
				}
				else
				{
					UE_LOG(LogTemp, Display,TEXT("Upgrade is called"));
					GetSpecificUpgradeFunction(Upgrade, Player);
				}
			}
		}
		// If you can't buy the product
		else
		{
			if (CantBuySound)
			{
				if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, Player->GetActorLocation());
				}
			}
		}
		
	}
	// If you own the product
	else
	{
		if (GetUpgradeCategory(Upgrade) == EUpgradeCategory::Weapon)
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				if (CanBuySound)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CanBuySound, Player->GetActorLocation());
				}
					
				SetCurrentWeapon(Upgrade);
				Player->SelectWeapon(*ConvertUpgradeTypeToString(Upgrade));
			}
		}
		else
		{
			if (CantBuySound)
			{
				if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, Player->GetActorLocation());
				}
			}
		}
	}
}

/*int32 UPlayerGameInstance::GetUpgradeCost(const FName Upgrade) const
{
}*/

FName UPlayerGameInstance::GetArrayName()
{
	FString CombinedString;
	for (EUpgradeType Upgrade : UpgradeArray)
	{
		CombinedString += ConvertUpgradeTypeToString(Upgrade) + ", ";
	}
	return FName(*CombinedString);
}

FName UPlayerGameInstance::GetCurrentWeaponName()
{
	return FName(*ConvertUpgradeTypeToString(CurrentWeapon));
}

void UPlayerGameInstance::SetCurrentWeapon(const EUpgradeType Weapon)
{
	CurrentWeapon = Weapon;
}

void UPlayerGameInstance::SetCurrentWeapon(const FName Weapon)
{
	for (EUpgradeType UpgradeType : UpgradeArray)
	{
		if (Weapon == ConvertUpgradeTypeToString(UpgradeType))
		{
			CurrentWeapon = UpgradeType;
		}
	}
}

void UPlayerGameInstance::GetAllUpgradeFunctions(APlayerCharacter* Player)
{
	UE_LOG(LogTemp,Warning,TEXT("AllUpgrade"));
	for (const EUpgradeType Upgrade : UpgradeArray)
	{
		if (Player != nullptr)
		{
			UseUpgradeFunction(Upgrade, Player);
		}
	}
}

void UPlayerGameInstance::GetSpecificUpgradeFunction(const EUpgradeType Upgrade, APlayerCharacter* Player)
{
	if (Player != nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("SpecificUpgrade"));
		UseUpgradeFunction(Upgrade, Player);
	}
}

void UPlayerGameInstance::UseUpgradeFunction(const EUpgradeType Upgrade, APlayerCharacter* Player)
{
	if (Player != nullptr)
	{
		switch (Upgrade)
		{
		case EUpgradeType::Health20:
			Player->PlayerHealth += 20;
			break;
		case EUpgradeType::Speed20:
			Player->GetCharacterMovement()->MaxWalkSpeed *= 1.2;
			break;
		case EUpgradeType::Jump50:
			Player->GetCharacterMovement()->JumpZVelocity *= 1.5;
			break;
		default:
			break;
		}
	}
}

FString UPlayerGameInstance::ConvertUpgradeTypeToString(const EUpgradeType Type)
{
	FString EnumString = StaticEnum<EUpgradeType>()->GetNameStringByValue(static_cast<int64>(Type));
	EnumString.RemoveFromStart(TEXT("EUpgradeType::"));
	return EnumString;
}
UPlayerGameInstance::UPlayerGameInstance()
{
	// Sätter upp level order, aka vilken level är numer 1 o.s.v
	LevelOrder = {
		FName("Hub"),
		FName("Arena_1"),
		FName("V1"),
		FName("V2"),
		FName("MetroV2")
	};

	// Låser upp level 1 och Huben direkt.
	if (LevelOrder.Num() > 0)
	{
		UnlockedLevels.Add(LevelOrder[0]);
		UnlockedLevels.Add(LevelOrder[1]);
	}
}
