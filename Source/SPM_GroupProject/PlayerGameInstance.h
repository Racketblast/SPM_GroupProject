// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PlayerGameInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	None        UMETA(DisplayName = "None"),
	Pistol        UMETA(DisplayName = "Pistol"),
	Rifle    UMETA(DisplayName = "Rifle"),
	Health20    UMETA(DisplayName = "20 Health"),
	Speed20  UMETA(DisplayName = "Speed 20%"),
	Jump50    UMETA(DisplayName = "50% Jump"),
	// Add more as needed
};

UENUM(BlueprintType)
enum class EUpgradeCategory : uint8
{
	None        UMETA(DisplayName = "None"),
	PlayerStats    UMETA(DisplayName = "Player Stats"),
	PlayerAbilities    UMETA(DisplayName = "Player Abilities"),
	Weapon    UMETA(DisplayName = "Weapon"),
	WeaponStats  UMETA(DisplayName = "Weapon Stats")
};

UCLASS()
class SPM_GROUPPROJECT_API UPlayerGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPlayerGameInstance();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Money;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWave;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUpgradeType CurrentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EUpgradeType> UpgradeArray;

	UFUNCTION(BlueprintCallable)
	bool HasBought(const EUpgradeType Upgrade) const;
	bool HasBought(const FName Upgrade) const;

	UFUNCTION(BlueprintCallable)
	FName GetArrayName();
	
	// För att låsa upp levels, relaterat till MissionSubsystem
	// Sätter upp level order, aka vilken level är numer 1 o.s.v
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progression")
	TArray<FName> LevelOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Progression")
	TSet<FName> UnlockedLevels;

	UFUNCTION(BlueprintCallable)
	FName GetCurrentWeaponName();
	
	UFUNCTION(BlueprintCallable)
	void SetCurrentWeapon(const EUpgradeType Weapon);
	void SetCurrentWeapon(const FName Weapon);
	
	UFUNCTION(BlueprintCallable)
	void GetAllUpgradeFunctions(class APlayerCharacter* Player = nullptr);

	UFUNCTION(BlueprintCallable)
	void GetSpecificUpgradeFunction(EUpgradeType Upgrade, class APlayerCharacter* Player = nullptr);


private:
	void UseUpgradeFunction(const EUpgradeType Upgrade, class APlayerCharacter* Player = nullptr);
	
	FString ConvertUpgradeTypeToString(const EUpgradeType Upgrade);
	
};
