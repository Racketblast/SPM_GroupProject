// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UpgradeEnums.h"
#include "UpgradeInfo.generated.h"

USTRUCT(BlueprintType)
struct FUpgradeInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EUpgradeCategory UpgradeCategory = EUpgradeCategory::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UpgradeCost = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UpgradeOwned = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 TotalUpgradeOwned = 1;
	
	FUpgradeInfo() {}

	FUpgradeInfo(EUpgradeCategory InCategory, int32 InCost, int32 InOwned = 0, int32 InTotalOwned = 1)
		: UpgradeCategory(InCategory), UpgradeCost(InCost), UpgradeOwned(InOwned), TotalUpgradeOwned(InTotalOwned)
	{}
};
