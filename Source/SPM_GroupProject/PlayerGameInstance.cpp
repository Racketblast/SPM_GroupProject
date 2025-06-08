// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerGameInstance.h"

#include "DialogueInfo.h"
#include "Pistol.h"
#include "PlayerCharacter.h"
#include "ProjectileGun.h"
#include "SwarmedSaveGame.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Map.h"

bool UPlayerGameInstance::HasBought(const EUpgradeType Upgrade) const
{
	return UpgradeMap.Contains(Upgrade);
}

bool UPlayerGameInstance::HasBought(const FName Upgrade) const
{
	for (const TPair<EUpgradeType, FUpgradeInfo>& UpgradeType : UpgradeMap)
	{
		if (FName(*StaticEnum<EUpgradeType>()->GetNameStringByValue(static_cast<int64>(UpgradeType.Key))) == Upgrade)
		{
			return true;
		}
	}
	return false;
}

FUpgradeInfo UPlayerGameInstance::SetDefaultUpgradeInfo(const EUpgradeType Upgrade)
{
	if (!UpgradeDataTable)
	return {};
	
	FName RowName(*ConvertUpgradeTypeToString(Upgrade));
	if (RowName != "")
	{
		if (FUpgradeInfo* Row = UpgradeDataTable->FindRow<FUpgradeInfo>(RowName, TEXT("")))
		{
			return *Row;
		}
	}
	
	return {};
}




void UPlayerGameInstance::BuyUpgrade(const EUpgradeType Upgrade, USoundBase* CanBuySound, USoundBase* CantBuySound)
{
	//Caches the player
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	
	// Gets the info from the object in the map if you have it, if not then it will use the default values
	FUpgradeInfo* UpgradeInfo = UpgradeMap.Find(Upgrade);
	FUpgradeInfo TempInfo;
	if (!UpgradeInfo)
	{
		TempInfo = SetDefaultUpgradeInfo(Upgrade);
		UpgradeInfo = &TempInfo;
	}
	
	// If you don't own the product or all of the possible to buy
	if (UpgradeInfo->UpgradeOwned < UpgradeInfo->TotalUpgradeOwned)
	{
		// If you can buy the product
		if (UpgradeInfo->UpgradeCosts.IsValidIndex(UpgradeInfo->UpgradeOwned))
		{
			if (UpgradeInfo->UpgradeCosts[UpgradeInfo->UpgradeOwned] <= Money)
			{
				PlayBuySound(CanBuySound, Player);
				
				Money -= UpgradeInfo->UpgradeCosts[UpgradeInfo->UpgradeOwned];
				UpgradeInfo->UpgradeOwned++;
				//Adds the upgrade to the map if it is not in there 
				if (!UpgradeMap.Contains(Upgrade))
				{
					UpgradeMap.Add(Upgrade, *UpgradeInfo);
				}
			
				if (Player)
				{
					if (UpgradeInfo->UpgradeCategory == EUpgradeCategory::Weapon)
					{
						BuyWeapon(Upgrade);
						SetCurrentWeapon(Upgrade);
						Player->SelectWeapon(*ConvertUpgradeTypeToString(Upgrade));
					}
					else
					{
						UE_LOG(LogTemp, Display,TEXT("Upgrade is called"));
						UseUpgradeFunction(Upgrade, Player);
					}
				}
			}
		}
		// If you can't buy the product
		else
		{
			PlayBuySound(CantBuySound, Player);
		}
		
	}
	// If you own the product
	else
	{
		//Switch weapons
		if (UpgradeInfo->UpgradeCategory == EUpgradeCategory::Weapon)
		{
			if (Player)
			{
				PlayBuySound(CanBuySound, Player);
					
				SetCurrentWeapon(Upgrade);
				Player->SelectWeapon(*ConvertUpgradeTypeToString(Upgrade));
			}
		}
		//Can't switch weapons
		else
		{
			PlayBuySound(CantBuySound, Player);
		}
	}
}

void UPlayerGameInstance::PlayBuySound(USoundBase* Sound, const APlayerCharacter* Player) const
{
	if (Sound)
	{
		if (Player)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Player->GetActorLocation());
		}
	}
}

void UPlayerGameInstance::BuyWeapon(EUpgradeType Weapon)
{
	switch (Weapon)
	{
	case EUpgradeType::Pistol:
		BuyUpgrade(EUpgradeType::PistolDamage);
		BuyUpgrade(EUpgradeType::PistolFiringSpeed);
		BuyUpgrade(EUpgradeType::PistolAmmoSize);
		break;
	case EUpgradeType::Rifle:
		BuyUpgrade(EUpgradeType::RifleDamage);
		BuyUpgrade(EUpgradeType::RifleFiringSpeed);
		BuyUpgrade(EUpgradeType::RifleAmmoSize);
		break;
	case EUpgradeType::Shotgun:
		BuyUpgrade(EUpgradeType::ShotgunDamage);
		BuyUpgrade(EUpgradeType::ShotgunFiringSpeed);
		BuyUpgrade(EUpgradeType::ShotgunAmmoSize);
		break;
	case EUpgradeType::RocketLauncher:
		BuyUpgrade(EUpgradeType::RocketLauncherDamage);
		BuyUpgrade(EUpgradeType::RocketLauncherFiringSpeed);
		BuyUpgrade(EUpgradeType::RocketLauncherAmmoSize);
		break;
	default:
		break;
	}
}

FName UPlayerGameInstance::GetArrayName()
{
	FString CombinedString;
	for (const TPair<EUpgradeType, FUpgradeInfo>& Upgrade : UpgradeMap)
	{
		CombinedString += ConvertUpgradeTypeToString(Upgrade.Key) + ", ";
	}
	return FName(*CombinedString);
}

FName UPlayerGameInstance::GetCurrentWeaponName()
{
	return FName(*ConvertUpgradeTypeToString(CurrentWeapon));
}

void UPlayerGameInstance::SetCurrentWeapon(const EUpgradeType Weapon)
{
	CurrentWeapon = Weapon;
}

void UPlayerGameInstance::SetCurrentWeapon(const FName Weapon)
{
	for (const TPair<EUpgradeType, FUpgradeInfo>& UpgradeType : UpgradeMap)
	{
		if (Weapon == ConvertUpgradeTypeToString(UpgradeType.Key))
		{
			CurrentWeapon = UpgradeType.Key;
		}
	}
}

FUpgradeInfo UPlayerGameInstance::GetUpgradeInfo(const EUpgradeType Upgrade) const
{
	if (const FUpgradeInfo* Info = UpgradeMap.Find(Upgrade))
		return *Info;
	return {};
}

void UPlayerGameInstance::ApplyAllUpgradeFunctions(APlayerCharacter* Player)
{
	if (!Player)
		return;
	// Apply upgrades to the player
	for (const TPair<EUpgradeType, FUpgradeInfo>& Upgrade : UpgradeMap)
	{
		UseUpgradeFunction(Upgrade.Key, Player);
	}
}

void UPlayerGameInstance::UseUpgradeFunction(const EUpgradeType Upgrade, APlayerCharacter* Player)
{
	if (Player != nullptr)
	{
		if (FUpgradeInfo* UpgradeInfo = UpgradeMap.Find(Upgrade))
		{
			switch (UpgradeInfo->UpgradeCategory)
			{
			case EUpgradeCategory::Weapon:
				break;
			case EUpgradeCategory::PlayerAbilities:
				break;
			case EUpgradeCategory::PlayerStats:
				UpgradePlayerStats(Upgrade, Player);
				break;
			case EUpgradeCategory::WeaponStats:
				UpgradeGunStats(Upgrade, Player);
				break;
			default:
				break;
			}
		}
	}
}

void UPlayerGameInstance::UpgradePlayerStats(const EUpgradeType Upgrade, class APlayerCharacter* Player)
{
	FUpgradeInfo* UpgradeInfo = UpgradeMap.Find(Upgrade);
	switch (Upgrade)
	{
	case EUpgradeType::Health:
		Player->PlayerMaxHealth = Player->BasePlayerMaxHealth + UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1];
		Player->PlayerHealth = Player->PlayerMaxHealth;
		break;
	case EUpgradeType::HealthMax:
		Player->PlayerMaxHealth = 1000000;
		Player->PlayerHealth = Player->PlayerMaxHealth;
		break;
	case EUpgradeType::Speed:
		Player->GetCharacterMovement()->MaxWalkSpeed = Player->BasePlayerMaxHealth + UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1];
		break;
	default:
		break;
	}
}

void UPlayerGameInstance::UpgradeGunStats(const EUpgradeType Upgrade, class APlayerCharacter* Player)
{
	FUpgradeInfo* UpgradeInfo = UpgradeMap.Find(Upgrade);

	AGun* Pistol = Player->GetWeaponInstance(EUpgradeType::Pistol);
	AGun* Rifle = Player->GetWeaponInstance(EUpgradeType::Rifle);
	AGun* Shotgun = Player->GetWeaponInstance(EUpgradeType::Shotgun);
	AGun* RocketLauncher = Player->GetWeaponInstance(EUpgradeType::RocketLauncher);
	
	if (Player->CurrentGun)
	{
		switch (Upgrade)
		{
			//Damage
		case EUpgradeType::PistolDamage:
			UpgradeGunStatValue(Player, Pistol, Pistol->WeaponDamage, UpgradeInfo);
			break;
		case EUpgradeType::RifleDamage:
			UpgradeGunStatValue(Player, Rifle, Rifle->WeaponDamage, UpgradeInfo);
			break;
		case EUpgradeType::ShotgunDamage:
			UpgradeGunStatValue(Player, Shotgun, Shotgun->WeaponDamage, UpgradeInfo);
			break;
		case EUpgradeType::RocketLauncherDamage:
			UpgradeGunStatValue(Player, RocketLauncher, RocketLauncher->WeaponDamage, UpgradeInfo);
			break;	

			//Firing speed
		case EUpgradeType::PistolFiringSpeed:
			UpgradeGunStatValue(Player, Pistol, Pistol->RoundsPerSecond, UpgradeInfo);
			break;
		case EUpgradeType::RifleFiringSpeed:
			UpgradeGunStatValue(Player, Rifle, Rifle->RoundsPerSecond, UpgradeInfo);
			break;
		case EUpgradeType::ShotgunFiringSpeed:
			UpgradeGunStatValue(Player,Shotgun, Shotgun->RoundsPerSecond, UpgradeInfo);
			break;
		case EUpgradeType::RocketLauncherFiringSpeed:
			UpgradeGunStatValue(Player, RocketLauncher, RocketLauncher->RoundsPerSecond, UpgradeInfo);
			break;
			
			//Ammo Size
		case EUpgradeType::PistolAmmoSize:
			UpgradeGunStatAmmo(Player, Pistol, UpgradeInfo);
			break;
		case EUpgradeType::RifleAmmoSize:
			UpgradeGunStatAmmo(Player, Rifle, UpgradeInfo);
			break;
		case EUpgradeType::ShotgunAmmoSize:
			UpgradeGunStatAmmo(Player, Shotgun, UpgradeInfo);
			break;
		case EUpgradeType::RocketLauncherAmmoSize:
			UpgradeGunStatAmmo(Player, RocketLauncher, UpgradeInfo);
			break;
		case EUpgradeType::GrenadesAmmoSize:
			Player->GrenadeNum = UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1];
			break;

			//Skins
			case EUpgradeType::PistolSkin:
			UpgradeGunSkin(Player, Pistol, 0);
			break;
		case EUpgradeType::RifleSkin:
			UpgradeGunSkin(Player, Rifle, 1);
			break;
		case EUpgradeType::ShotgunSkin:
			UpgradeGunSkin(Player, Shotgun, 2);
			break;
		case EUpgradeType::RocketLauncherSkin:
			UpgradeGunSkin(Player, RocketLauncher, 3);
			break;
		default:
			break;
		}
	}
}

void UPlayerGameInstance::UpgradeGunStatValue(APlayerCharacter* Player, AGun* Weapon, float& ValueToChange, FUpgradeInfo* UpgradeInfo)
{
	if (Player->CurrentGun == Weapon)
	{
		Player->CurrentGun->bIsUpgraded = true;
		ValueToChange = UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1];
	}
}

void UPlayerGameInstance::UpgradeGunStatAmmo(APlayerCharacter* Player, AGun* Weapon, FUpgradeInfo* UpgradeInfo)
{
	if (Player->CurrentGun == Weapon)
	{
		if (Cast<APistol>(Weapon))
		{
			Weapon->TotalAmmo = UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1];
			Weapon->MaxTotalAmmo = UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1];
		}
		else
		{
			Weapon->TotalAmmo = UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1]*5;
			Weapon->MaxTotalAmmo = UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1]*5;
		}
		
		Weapon->MaxAmmo = UpgradeInfo->UpgradeValues[UpgradeInfo->UpgradeOwned-1];
		Weapon->CurrentAmmo = Weapon->MaxAmmo;
		Player->CurrentGun->bIsUpgraded = true;
	}
}

void UPlayerGameInstance::UpgradeGunSkin(APlayerCharacter* Player, AGun* Weapon, int32 WeaponSkinIndex)
{
	
	if (Player->CurrentGun == Weapon)
	{
		if (bSwapMaterials)
		{
			CurrentWeaponSkins[WeaponSkinIndex] = 1 - CurrentWeaponSkins[WeaponSkinIndex];
			bSwapMaterials = false;
		}
		if (Weapon->DifferentSkinMat.IsValidIndex(CurrentWeaponSkins[WeaponSkinIndex]))
		{
			if (Weapon == Player->GetWeaponInstance("Rifle"))
			{
				Weapon->WeaponSkeletalMesh->SetMaterial(1,Weapon->DifferentSkinMat[CurrentWeaponSkins[WeaponSkinIndex]]);
			}
			else
			{
				Weapon->WeaponSkeletalMesh->SetMaterial(0,Weapon->DifferentSkinMat[CurrentWeaponSkins[WeaponSkinIndex]]);
			}
		}
		Player->CurrentGun->bIsUpgraded = true;
	}
}

FString UPlayerGameInstance::ConvertUpgradeTypeToString(const EUpgradeType Type)
{
	FString EnumString = StaticEnum<EUpgradeType>()->GetNameStringByValue(static_cast<int64>(Type));
	EnumString.RemoveFromStart(TEXT("EUpgradeType::"));
	return EnumString;
}

void UPlayerGameInstance::Init()
{
	Super::Init();
	
	// Sätter upp level order, aka vilken level är numer 1 o.s.v
	LevelOrder = {
		FName("Hub"),
		FName("Arena_1"),
		FName("V1"),
		FName("V3"),
		FName("MetroV3")
	};
	
	// Låser upp level 1 och Huben direkt.
	if (LevelOrder.Num() > 0)
	{
		UnlockedLevels.Add(LevelOrder[0]);
		UnlockedLevels.Add(LevelOrder[1]);
		UnlockedLevels.Add(LevelOrder[2]);
		UnlockedLevels.Add(LevelOrder[3]);
	}
	
	//Loads the saved game
	LoadGame();
}

void UPlayerGameInstance::SaveGame()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		FillSaveGame();
		UGameplayStatics::SaveGameToSlot(Save,"Save1", 0);
	}
	else if (SaveGameObject)
	{
		Save = Cast<USwarmedSaveGame>(UGameplayStatics::CreateSaveGameObject(SaveGameObject));
		if (Save)
		{
			FillSaveGame();
			UGameplayStatics::SaveGameToSlot(Save,"Save1", 0);
		}
	}
}

void UPlayerGameInstance::FillSaveGame()
{
	//Saved stats
	Save->SavedMoney = Money;
	Save->SavedUpgradeMap = UpgradeMap;
	Save->SavedCurrentWeapon = CurrentWeapon;
	Save->SavedUnlockedLevels = UnlockedLevels;
	Save->SavedCurrentGameFlag = CurrentGameFlag;
	Save->SavedCurrentSkins = CurrentWeaponSkins;

	//Saved options
	Save->SavedMouseSensitivityScale = MouseSensitivityScale;
	Save->SavedMasterVolumeScale = MasterVolumeScale;
	Save->SavedSFXVolumeScale = SFXVolumeScale;
	Save->SavedMusicVolumeScale = MusicVolumeScale;
}

void UPlayerGameInstance::LoadGame()
{
	//Loads the saved game
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Save = Cast<USwarmedSaveGame>(UGameplayStatics::LoadGameFromSlot("Save1",0));
		if (Save)
		{
			Money = Save->SavedMoney;
			UpgradeMap = Save->SavedUpgradeMap;
			CurrentWeapon = Save->SavedCurrentWeapon;
			UnlockedLevels = Save->SavedUnlockedLevels;
			CurrentGameFlag = Save->SavedCurrentGameFlag;
			CurrentWeaponSkins = Save->SavedCurrentSkins;
			
			MouseSensitivityScale = Save->SavedMouseSensitivityScale;
			MasterVolumeScale = Save->SavedMasterVolumeScale;
			SFXVolumeScale = Save->SavedSFXVolumeScale;
			MusicVolumeScale = Save->SavedMusicVolumeScale;
		}
	}
}

void UPlayerGameInstance::RestartGame()
{
	Money = 0;
	UpgradeMap = {};
	UnlockedLevels = {};
	CurrentWeapon = EUpgradeType::None;
	UnlockedLevels.Add(LevelOrder[0]);
	UnlockedLevels.Add(LevelOrder[1]);
	UnlockedLevels.Add(LevelOrder[2]);
	UnlockedLevels.Add(LevelOrder[3]);
	CurrentGameFlag = 0;
}

bool UPlayerGameInstance::HasGameChanged()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Save = Cast<USwarmedSaveGame>(UGameplayStatics::LoadGameFromSlot("Save1",0));
		if (Save)
		{
			if (Money != Save->SavedMoney)
				return true;
			if (UpgradeMap.Num() != Save->SavedUpgradeMap.Num())
				return true;
			for (const TPair<EUpgradeType, FUpgradeInfo>& UpgradeType : UpgradeMap)
			{
				const FUpgradeInfo* ValueB = Save->SavedUpgradeMap.Find(UpgradeType.Key);
				if (!ValueB || *ValueB != UpgradeType.Value)
					return true;
			}
			if (CurrentWeapon != Save->SavedCurrentWeapon)
				return true;
			if (!UnlockedLevels.Includes(Save->SavedUnlockedLevels) || !Save->SavedUnlockedLevels.Includes(UnlockedLevels))
				return true;
			if (CurrentGameFlag != Save->SavedCurrentGameFlag)
				return true;
			if (CurrentWeaponSkins != Save->SavedCurrentSkins)
				return true;

			if (MouseSensitivityScale != Save->SavedMouseSensitivityScale)
				return true;
			if (MasterVolumeScale != Save->SavedMasterVolumeScale)
				return true;
			if (SFXVolumeScale != Save->SavedSFXVolumeScale)
				return true;
			if (MusicVolumeScale != Save->SavedMusicVolumeScale)
				return true;
		}
		return false;
	}
	
	return true;
}

void UPlayerGameInstance::StartDialogue()
{
	if (!EventDialogueInfo)
		return;

	if (!bCanPlayDialogue)
		return;

	
	if (StartDialogueRowName == "")
		return;
	
	CurrentDialogueRowName = StartDialogueRowName;
	
	if (FDialogueInfo* Row = EventDialogueInfo->FindRow<FDialogueInfo>(StartDialogueRowName, TEXT("")))
	{
		if (CurrentGameFlag < Row->DialogueFlag || Row->DialogueFlag == 0)
		{
			NextDialogueRowName = Row->NextDialogue;

			//Plays the dialogue for the amount of time the sound plays
			float TimeUntilNextDialogue = 0.0f;
			if ( APawn* Player = Cast<APawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
			{
				if (Row->DialogueSound)
				{
					bDialogueIsPlaying = true;
					DialogueComponent = UGameplayStatics::SpawnSoundAtLocation(
						GetWorld(),
						Row->DialogueSound,
						Player->GetActorLocation()
					);
					TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
				}
			}
		
			//Goes to next dialogue
			//This is the reason why the dialogue is broken into two functions, because it needs a delay between each dialog
			GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UPlayerGameInstance::PlayNextDialogue, TimeUntilNextDialogue, false);
		}
	}
}

void UPlayerGameInstance::PlayNextDialogue()
{
	if (!EventDialogueInfo)
	return;
	if (NextDialogueRowName != "")
	{
		CurrentDialogueRowName = NextDialogueRowName;
		if (FDialogueInfo* Row = EventDialogueInfo->FindRow<FDialogueInfo>(NextDialogueRowName, TEXT("")))
		{
			NextDialogueRowName = Row->NextDialogue;
			//Plays the dialogue for the amount of time the sound plays
			float TimeUntilNextDialogue = 0.0f;
			if ( APawn* Player = Cast<APawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
			{
				if (Row->DialogueSound)
				{
					DialogueComponent = UGameplayStatics::SpawnSoundAtLocation(
						GetWorld(),
						Row->DialogueSound,
						Player->GetActorLocation()
					);
					TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
				}
			}
			
			//Goes to next dialogue
			GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UPlayerGameInstance::PlayNextDialogue, TimeUntilNextDialogue, false);
		}
	}
	//If dialogue is over, take away the widgets
	else
	{
		bDialogueIsPlaying = false;
	}
}

void UPlayerGameInstance::StopDialogue()
{
	GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);

	if (DialogueComponent && DialogueComponent->IsPlaying())
	{
		DialogueComponent->Stop();
	}

	bDialogueIsPlaying = false;
	StartDialogueRowName = "";
	CurrentDialogueRowName = "";
	NextDialogueRowName = "";
}
