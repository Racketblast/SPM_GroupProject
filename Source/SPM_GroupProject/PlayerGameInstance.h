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
	None                UMETA(DisplayName = "None"),
	Pistol              UMETA(DisplayName = "Pistol"),
	Rifle               UMETA(DisplayName = "Rifle"),
	Shotgun             UMETA(DisplayName = "Shotgun"),
	RocketLauncher      UMETA(DisplayName = "RocketLauncher"),
	Health20            UMETA(DisplayName = "20 Health"),
	HealthMax           UMETA(DisplayName = "Max Health"),
	Speed20             UMETA(DisplayName = "Speed 20%"),
	Jump50              UMETA(DisplayName = "50% Jump"),
	PistolDamage10      UMETA(DisplayName = "10% Pistol Damage"),
	RifleDamage10       UMETA(DisplayName = "10% Rifle Damage"),
	PistolFiringSpeed10 UMETA(DisplayName = "10% Pistol Firing Speed"),
	RifleFiringSpeed10  UMETA(DisplayName = "10% Rifle Firing Speed"),
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

USTRUCT(BlueprintType)
struct FUpgradeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EUpgradeCategory UpgradeCategory = EUpgradeCategory::None;
	UPROPERTY(BlueprintReadOnly)
	int32 UpgradeCost = 0;
	UPROPERTY(BlueprintReadOnly)
	int32 UpgradeOwned = 1;
	UPROPERTY(BlueprintReadOnly)
	int32 TotalUpgradeOwned = 1;
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
	TMap<EUpgradeType,FUpgradeInfo> UpgradeMap;

	UFUNCTION(BlueprintCallable)
	bool HasBought(const EUpgradeType Upgrade) const;
	bool HasBought(const FName Upgrade) const;

	UFUNCTION(BlueprintCallable)
	FUpgradeInfo SetDefaultUpgradeInfo(const EUpgradeType Upgrade) const;
	UFUNCTION(BlueprintCallable)
	void BuyUpgrade(const EUpgradeType Upgrade,USoundBase* CanBuySound = nullptr, USoundBase* CantBuySound = nullptr);

	UFUNCTION(BlueprintCallable)
	FName GetArrayName();
	
	// För att låsa upp levels, relaterat till MissionSubsystem
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progression")
	TArray<FName> LevelOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Progression")
	TSet<FName> UnlockedLevels;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Progression")
	FName LastCompletedLevel;

	UFUNCTION(BlueprintCallable)
	FName GetCurrentWeaponName();
	
	UFUNCTION(BlueprintCallable)
	void SetCurrentWeapon(const EUpgradeType Weapon);
	void SetCurrentWeapon(const FName Weapon);

	UFUNCTION(BlueprintCallable)
	FUpgradeInfo GetUpgradeInfo(const EUpgradeType Weapon);
	
	UFUNCTION(BlueprintCallable)
	void ApplyAllUpgradeFunctions(class APlayerCharacter* Player);

	
	void UseUpgradeFunction(const EUpgradeType Upgrade, class APlayerCharacter* Player);
	void UpgradePlayerStats(const EUpgradeType Upgrade, class APlayerCharacter* Player);
	void UpgradeGunStats(const EUpgradeType Upgrade, class APlayerCharacter* Player);
private:
	
	FString ConvertUpgradeTypeToString(const EUpgradeType Upgrade);
};


