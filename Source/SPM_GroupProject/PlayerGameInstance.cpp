// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerGameInstance.h"

bool UPlayerGameInstance::HasBought(FName Upgrade)
{
	for (FName OwnedUpgrades : UpgradeArray)
	{
		if (OwnedUpgrades == Upgrade)
		{
			return true;
		}
	}
	return false;
}

FName UPlayerGameInstance::GetArrayName()
{
	FString CombinedString;
	for (FName Name : UpgradeArray)
	{
		CombinedString += Name.ToString() + ", ";
	}
	return FName(*CombinedString);
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