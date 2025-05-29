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
	DoomsdayGun			UMETA(DisplayName = "DoomsdayGun"),
	Health            UMETA(DisplayName = "Health"),
	HealthMax           UMETA(DisplayName = "Max Health"),
	Speed             UMETA(DisplayName = "Speed"),
	PistolDamage      UMETA(DisplayName = "Pistol Damage"),
	RifleDamage       UMETA(DisplayName = "Rifle Damage"),
	ShotgunDamage       UMETA(DisplayName = "Shotgun Damage"),
	RocketLauncherDamage       UMETA(DisplayName = "RocketLauncher Damage"),
	PistolFiringSpeed UMETA(DisplayName = "Pistol Firing Speed"),
	RifleFiringSpeed  UMETA(DisplayName = "Rifle Firing Speed"),
	ShotgunFiringSpeed UMETA(DisplayName = "Shotgun Firing Speed"),
	RocketLauncherFiringSpeed UMETA(DisplayName = "RocketLauncher Firing Speed"),
	PistolAmmoSize UMETA(DisplayName = "Pistol Ammo Size"),
	RifleAmmoSize  UMETA(DisplayName = "Rifle Ammo Size"),
	ShotgunAmmoSize UMETA(DisplayName = "Shotgun Ammo Size"),
	RocketLauncherAmmoSize UMETA(DisplayName = "RocketLauncher Ammo Size"),
	GrenadesAmmoSize UMETA(DisplayName = "Grenades Ammo Size"),
	PistolSkin UMETA(DisplayName = "Pistol Skin"),
	RifleSkin  UMETA(DisplayName = "Rifle Skin"),
	ShotgunSkin UMETA(DisplayName = "Shotgun Skin"),
	RocketLauncherSkin UMETA(DisplayName = "RocketLauncher Skin"),
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