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
