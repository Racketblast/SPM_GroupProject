// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerGameInstance.h"

#include "DialogueInfo.h"
#include "PlayerCharacter.h"
#include "ProjectileGun.h"
#include "SwarmedSaveGame.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/SGameLayerManager.h"

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
	if (FUpgradeInfo* Row = UpgradeDataTable->FindRow<FUpgradeInfo>(RowName, TEXT("")))
	{
		return *Row;
	}
	
	return {};
}




void UPlayerGameInstance::BuyUpgrade(const EUpgradeType Upgrade, USoundBase* CanBuySound, USoundBase* CantBuySound)
{
	// Gets the info from the object in the map if you have it, if not then it will use the default values
	FUpgradeInfo* UpgradeInfo = UpgradeMap.Find(Upgrade);
	if (!UpgradeInfo)
	{
		FUpgradeInfo TempInfo = SetDefaultUpgradeInfo(Upgrade);
		UpgradeInfo = &TempInfo;
	}
	
	// If you don't own the product or all of the possible to buy
	if (UpgradeInfo->UpgradeOwned < UpgradeInfo->TotalUpgradeOwned)
	{
		// If you can buy the product
		if (UpgradeInfo->UpgradeCost <= Money)
		{
			if (CanBuySound)
			{
				if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CanBuySound, Player->GetActorLocation());
				}
			}
			Money -= UpgradeInfo->UpgradeCost;
			UpgradeInfo->UpgradeOwned++;
			UpgradeInfo->UpgradeCost *= 1.2;
			//Adds the upgrade to the map if it is not in there 
			if (!UpgradeMap.Contains(Upgrade))
			{
				UpgradeMap.Add(Upgrade, *UpgradeInfo);
			}
			
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				if (UpgradeInfo->UpgradeCategory == EUpgradeCategory::Weapon)
				{
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
		// If you can't buy the product
		else
		{
			if (CantBuySound)
			{
				if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, Player->GetActorLocation());
				}
			}
		}
		
	}
	// If you own the product
	else
	{
		//Switch weapons
		if (UpgradeInfo->UpgradeCategory == EUpgradeCategory::Weapon)
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				if (CanBuySound)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CanBuySound, Player->GetActorLocation());
				}
					
				SetCurrentWeapon(Upgrade);
				Player->SelectWeapon(*ConvertUpgradeTypeToString(Upgrade));
			}
		}
		//Can't switch weapons
		else
		{
			if (CantBuySound)
			{
				if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CantBuySound, Player->GetActorLocation());
				}
			}
		}
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
	for (const TPair<EUpgradeType, FUpgradeInfo>& UpgradeType: UpgradeMap)
	{
		if (UpgradeType.Key == Upgrade)
		{
			return UpgradeType.Value;
		}
	}
	return {EUpgradeCategory::None,0,0,0};
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
		FUpgradeInfo* UpgradeInfo = UpgradeMap.Find(Upgrade);
		if (UpgradeInfo)
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
	case EUpgradeType::Health20:
		Player->PlayerMaxHealth = Player->BasePlayerMaxHealth + 20 * UpgradeInfo->UpgradeOwned;
		Player->PlayerHealth = Player->PlayerMaxHealth;
		break;
	case EUpgradeType::HealthMax:
		Player->PlayerMaxHealth = 1000000;
		Player->PlayerHealth = Player->PlayerMaxHealth;
		break;
	case EUpgradeType::Speed20:
		Player->GetCharacterMovement()->MaxWalkSpeed *= 1 + 0.2 * UpgradeInfo->UpgradeOwned;
		break;
	case EUpgradeType::Jump50:
		Player->GetCharacterMovement()->JumpZVelocity *= 1 + 0.5 * UpgradeInfo->UpgradeOwned;
		break;
	default:
		break;
	}
}

void UPlayerGameInstance::UpgradeGunStats(const EUpgradeType Upgrade, class APlayerCharacter* Player)
{
	FUpgradeInfo* UpgradeInfo = UpgradeMap.Find(Upgrade);
	if (Player->CurrentGun)
	{
		switch (Upgrade)
		{
			//Damage
		case EUpgradeType::PistolDamage10:
			if (Player-> CurrentGun == Player->GetWeaponInstance("Pistol"))
			{
				Player->GetWeaponInstance("Pistol")->WeaponDamage = Player->GetWeaponInstance("Pistol")->BaseWeaponDamage * (1 + 0.1 * UpgradeInfo->UpgradeOwned);
				Player->CurrentGun->bIsUpgraded = true;
			}
			break;
		case EUpgradeType::RifleDamage10:
			if (Player-> CurrentGun == Player->GetWeaponInstance("Rifle"))
			{
				Player->GetWeaponInstance("Rifle")->WeaponDamage = Player->GetWeaponInstance("Rifle")->BaseWeaponDamage * (1 + 0.1 * UpgradeInfo->UpgradeOwned);
				Player->CurrentGun->bIsUpgraded = true;
			}
			break;
		case EUpgradeType::ShotgunDamage10:
			if (Player-> CurrentGun == Player->GetWeaponInstance("Shotgun"))
			{
				Player->GetWeaponInstance("Shotgun")->WeaponDamage = Player->GetWeaponInstance("Shotgun")->BaseWeaponDamage * (1 + 0.1 * UpgradeInfo->UpgradeOwned);
				Player->CurrentGun->bIsUpgraded = true;
			}
			break;
		case EUpgradeType::RocketLauncherDamage10:
			if (Player-> CurrentGun == Player->GetWeaponInstance("RocketLauncher"))
			{
				Player->GetWeaponInstance("RocketLauncher")->WeaponDamage = Player->GetWeaponInstance("RocketLauncher")->BaseWeaponDamage * (1 + 0.1 * UpgradeInfo->UpgradeOwned);
				Player->CurrentGun->bIsUpgraded = true;
			}
			break;	

			//Firing speed
		case EUpgradeType::PistolFiringSpeed10:
			if (Player-> CurrentGun == Player->GetWeaponInstance("Pistol"))
			{
				Player->GetWeaponInstance("Pistol")->RoundsPerSecond = Player->GetWeaponInstance("Pistol")->BaseRoundsPerSecond * (1 + 0.1 * UpgradeInfo->UpgradeOwned);
				Player->CurrentGun->bIsUpgraded = true;
			}
			break;
		case EUpgradeType::RifleFiringSpeed10:
			if (Player-> CurrentGun == Player->GetWeaponInstance("Rifle"))
			{
				Player->GetWeaponInstance("Rifle")->RoundsPerSecond = Player->GetWeaponInstance("Rifle")->BaseRoundsPerSecond * (1 + 0.1 * UpgradeInfo->UpgradeOwned);
				Player->CurrentGun->bIsUpgraded = true;
			}
			break;
		case EUpgradeType::ShotgunFiringSpeed10:
			if (Player-> CurrentGun == Player->GetWeaponInstance("Shotgun"))
			{
				Player->GetWeaponInstance("Shotgun")->RoundsPerSecond = Player->GetWeaponInstance("Shotgun")->BaseRoundsPerSecond * (1 + 0.1 * UpgradeInfo->UpgradeOwned);
				Player->CurrentGun->bIsUpgraded = true;
			}
			break;
		case EUpgradeType::RocketLauncherFiringSpeed10:
			if (Player-> CurrentGun == Player->GetWeaponInstance("RocketLauncher"))
			{
				Player->GetWeaponInstance("RocketLauncher")->RoundsPerSecond = Player->GetWeaponInstance("RocketLauncher")->BaseRoundsPerSecond * (1 + 0.1 * UpgradeInfo->UpgradeOwned);
				Player->CurrentGun->bIsUpgraded = true;
			}
			break;
		default:
			break;
		}
	}
}

FString UPlayerGameInstance::ConvertUpgradeTypeToString(const EUpgradeType Type)
{
	FString EnumString = StaticEnum<EUpgradeType>()->GetNameStringByValue(static_cast<int64>(Type));
	EnumString.RemoveFromStart(TEXT("EUpgradeType::"));
	return EnumString;
}

UPlayerGameInstance::UPlayerGameInstance()
{
}

void UPlayerGameInstance::Init()
{
	Super::Init();
	
	// Sätter upp level order, aka vilken level är numer 1 o.s.v
	LevelOrder = {
		FName("Hub"),
		FName("Arena_1"),
		FName("V1"),
		FName("V2"),
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
		}
	}
	//Creates a new saved game if nothing exists
	else
	{
		if (SaveGameObject)
		{
			Save = Cast<USwarmedSaveGame>(UGameplayStatics::CreateSaveGameObject(SaveGameObject));
			if (Save)
			{
				Save->SavedMoney = Money;
				Save->SavedUpgradeMap = UpgradeMap;
				Save->SavedCurrentWeapon = CurrentWeapon;
				Save->SavedUnlockedLevels = UnlockedLevels;
				Save->SavedCurrentGameFlag = CurrentGameFlag;
				UGameplayStatics::SaveGameToSlot(Save,"Save1", 0);
			}
		}
	}
}

void UPlayerGameInstance::SaveGame()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Save->SavedMoney = Money;
		Save->SavedUpgradeMap = UpgradeMap;
		Save->SavedCurrentWeapon = CurrentWeapon;
		Save->SavedUnlockedLevels = UnlockedLevels;
		Save->SavedCurrentGameFlag = CurrentGameFlag;
		UGameplayStatics::SaveGameToSlot(Save,"Save1", 0);
	}
}

void UPlayerGameInstance::RestartGame()
{
	if (UGameplayStatics::DoesSaveGameExist("Save1",0))
	{
		Money = 0;
		UpgradeMap = {};
		UnlockedLevels = {};
		Save->SavedCurrentWeapon = EUpgradeType::None;
		UnlockedLevels.Add(LevelOrder[0]);
		UnlockedLevels.Add(LevelOrder[1]);
		UnlockedLevels.Add(LevelOrder[2]);
		UnlockedLevels.Add(LevelOrder[3]);
		CurrentGameFlag = 0;
	}
}

void UPlayerGameInstance::StartDialogue()
{
	if (!EventDialogueInfo)
		return;
	
	CurrentDialogueRowName = StartDialogueRowName;
	
	if (FDialogueInfo* Row = EventDialogueInfo->FindRow<FDialogueInfo>(StartDialogueRowName, TEXT("")))
	{
		if (CurrentGameFlag < Row->DialogueFlag || Row->DialogueFlag == 0)
		{
			if (CurrentGameFlag < Row->DialogueFlag)
			{
				CurrentGameFlag++;
			}
			NextDialogueRowName = Row->NextDialogue;

			//This is the reason why the dialogue iss broken into two functions, because I don't want to get an infinite amount of widgets
			if (Row->DialogueWidgetClass)
			{
				if (UUserWidget* DialogueWidget = CreateWidget<UUserWidget>(GetWorld(), Row->DialogueWidgetClass))
				{
					DialogueWidget->AddToViewport();
				}
			}

			//Plays the dialogue for the amount of time the sound plays
			float TimeUntilNextDialogue = 0.0f;
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				if (Row->DialogueSound)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), Row->DialogueSound, Player->GetActorLocation());
					TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
				}
			}
		
			//Goes to next dialogue
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPlayerGameInstance::PlayNextDialogue, TimeUntilNextDialogue, false);
		}
	}
}

void UPlayerGameInstance::PlayNextDialogue()
{
	if (!EventDialogueInfo)
	return;
	CurrentDialogueRowName = NextDialogueRowName;
	if (NextDialogueRowName != "")
	{
		if (FDialogueInfo* Row = EventDialogueInfo->FindRow<FDialogueInfo>(NextDialogueRowName, TEXT("")))
		{
			NextDialogueRowName = Row->NextDialogue;
			//Plays the dialogue for the amount of time the sound plays
			float TimeUntilNextDialogue = 0.0f;
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				if (Row->DialogueSound)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), Row->DialogueSound, Player->GetActorLocation());
					TimeUntilNextDialogue = Row->DialogueSound->GetDuration();
				}
			}
			
			//Goes to next dialogue
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPlayerGameInstance::PlayNextDialogue, TimeUntilNextDialogue, false);
		}
	}
	//If dialogue is over, take away the widgets
	else
	{
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UUserWidget::StaticClass(), false);
		if (FDialogueInfo* Row = EventDialogueInfo->FindRow<FDialogueInfo>(StartDialogueRowName, TEXT("")))
		{
			for (UUserWidget* Widget : FoundWidgets)
			{
				if (Widget && Widget->IsInViewport() && Widget->GetClass() == Row->DialogueWidgetClass)
				{
					Widget->RemoveFromParent();
				}
			}
		}
	}
}