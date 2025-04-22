// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PlayerGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API UPlayerGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Money;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWave;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> UpgradeArray;

	UFUNCTION(BlueprintCallable)
	bool HasBought(FName Upgrade);

	UFUNCTION(BlueprintCallable)
	FName GetArrayName();
};
