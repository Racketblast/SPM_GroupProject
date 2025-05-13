// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeEnums.generated.h"

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
	ShotgunDamage10       UMETA(DisplayName = "10% Shotgun Damage"),
	RocketLauncherDamage10       UMETA(DisplayName = "10% RocketLauncher Damage"),
	PistolFiringSpeed10 UMETA(DisplayName = "10% Pistol Firing Speed"),
	RifleFiringSpeed10  UMETA(DisplayName = "10% Rifle Firing Speed"),
	ShotgunFiringSpeed10 UMETA(DisplayName = "10% Shotgun Firing Speed"),
	RocketLauncherFiringSpeed10 UMETA(DisplayName = "10% RocketLauncher Firing Speed"),
	PistolAmmoSize UMETA(DisplayName = "Pistol Ammo Size"),
	RifleAmmoSize  UMETA(DisplayName = "Rifle Ammo Size"),
	ShotgunAmmoSize UMETA(DisplayName = "Shotgun Ammo Size"),
	RocketLauncherAmmoSize UMETA(DisplayName = "RocketLauncher Ammo Size"),
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