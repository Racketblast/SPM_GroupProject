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
	LevelOrder = {
		FName("Hub"),
		FName("Arena_1"),
		FName("V1"),
		FName("V2"),
		FName("MetroV2")
	};
	if (LevelOrder.Num() > 0)
	{
		UnlockedLevels.Add(LevelOrder[0]);
		UnlockedLevels.Add(LevelOrder[1]);
	}
}
