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
	TArray<int32> UpgradeCosts = {0};

	UPROPERTY(BlueprintReadOnly)
	int32 UpgradeCost = UpgradeCosts[0];

	UPROPERTY(BlueprintReadOnly)
	int32 UpgradeOwned = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 TotalUpgradeOwned = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<float> UpgradeValues = {0};
	
	FUpgradeInfo() {}

	FUpgradeInfo(EUpgradeCategory InCategory, TArray<int32> InCost = {0}, int32 InOwned = 0, int32 InTotalOwned = 1, TArray<int32> InUpgradeValues = {0})
		: UpgradeCategory(InCategory), UpgradeCosts(InCost), UpgradeOwned(InOwned), TotalUpgradeOwned(InTotalOwned), UpgradeValues(InUpgradeValues)
	{}

	bool operator==(const FUpgradeInfo& Other) const;
	bool operator!=(const FUpgradeInfo& Other) const;
};

inline bool FUpgradeInfo::operator==(const FUpgradeInfo& Other) const
{
	if (this->UpgradeCategory != Other.UpgradeCategory)
		return false;
	if (this->UpgradeCosts != Other.UpgradeCosts)
		return false;
	if (this->UpgradeCost != Other.UpgradeCost)
		return false;
	if (this->UpgradeOwned != Other.UpgradeOwned)
		return false;
	if (this->TotalUpgradeOwned != Other.TotalUpgradeOwned)
		return false;
	if (this->UpgradeValues != Other.UpgradeValues)
		return false;
	
	return true;
}

inline bool FUpgradeInfo::operator!=(const FUpgradeInfo& Other) const
{
	return !(*this==Other);
}
