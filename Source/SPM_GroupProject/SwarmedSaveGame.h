// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UpgradeEnums.h"
#include "UpgradeInfo.h"

#include "GameFramework/SaveGame.h"
#include "SwarmedSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API USwarmedSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SavedMoney;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUpgradeType SavedCurrentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EUpgradeType,FUpgradeInfo> SavedUpgradeMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SavedCurrentGameFlag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> SavedUnlockedLevels;
};
