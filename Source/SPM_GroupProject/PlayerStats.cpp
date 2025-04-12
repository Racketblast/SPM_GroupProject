// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStats.h"

bool UPlayerStats::HasBought(FName Upgrade)
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