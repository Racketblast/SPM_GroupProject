// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PlayerStats.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API UPlayerStats : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Money = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Level = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName CurrentWeapon;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FName> UpgradeArray;

	bool HasBought(FName Upgrade);
};
