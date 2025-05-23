// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "UpgradeEnums.h"
#include "UpgradeInfo.h"

#include "PlayerGameInstance.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API UPlayerGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPlayerGameInstance();
	virtual void Init() override;

	void OnPostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class USwarmedSaveGame> SaveGameObject;
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	USwarmedSaveGame* Save;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGameStarted = false;
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanPlayDialogue = false;
	UPROPERTY(BlueprintReadOnly)
	bool bDialogueIsPlaying = false;
	UPROPERTY(BlueprintReadWrite)
	FName CurrentDialogueRowName;
	UPROPERTY(BlueprintReadWrite)
	FName StartDialogueRowName;
	FName NextDialogueRowName;

	//CurrentGameFlag means, at what point am I in the game. If CurrentGameFlag is less than a number it means that it should play, but if it's bigger, then it should not play
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentGameFlag = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	UDataTable* UpgradeDataTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	UDataTable* EventDialogueInfo;

	//For savegame
	UFUNCTION(BlueprintCallable)
	void SaveGame();
	UFUNCTION(BlueprintCallable)
	void LoadGame();
	UFUNCTION(BlueprintCallable)
	void RestartGame();
	UFUNCTION(BlueprintCallable)
	bool HasGameChanged();

	//Options
	UPROPERTY(BlueprintReadWrite)
	float MouseSensitivityScale = 1.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class USoundMix* SoundMix;
	UPROPERTY(EditDefaultsOnly)
	class USoundClass* MasterSoundClass;
	UPROPERTY(EditDefaultsOnly)
	class USoundClass* SFXSoundClass;
	UPROPERTY(EditDefaultsOnly)
	class USoundClass* MusicSoundClass;
	UPROPERTY(BlueprintReadWrite)
	float MasterVolumeScale = 1.0f;
	UPROPERTY(BlueprintReadWrite)
	float SFXVolumeScale = 1.0f;
	UPROPERTY(BlueprintReadWrite)
	float MusicVolumeScale = 1.0f;
	
	
	UFUNCTION(BlueprintCallable)
	bool HasBought(const EUpgradeType Upgrade) const;
	bool HasBought(const FName Upgrade) const;

	UFUNCTION(BlueprintCallable)
	FUpgradeInfo SetDefaultUpgradeInfo(const EUpgradeType Upgrade);
	UFUNCTION(BlueprintCallable)
	void BuyUpgrade(const EUpgradeType Upgrade,USoundBase* CanBuySound = nullptr, USoundBase* CantBuySound = nullptr);

	UFUNCTION(BlueprintCallable)
	FName GetArrayName();
	
	// För att låsa upp levels, relaterat till MissionSubsystem
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progression")
	TArray<FName> LevelOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Level Progression")
	TSet<FName> UnlockedLevels;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Progression")
	FName LastCompletedLevel;

	UFUNCTION(BlueprintCallable)
	FName GetCurrentWeaponName();
	
	UFUNCTION(BlueprintCallable)
	void SetCurrentWeapon(const EUpgradeType Weapon);
	void SetCurrentWeapon(const FName Weapon);

	UFUNCTION(BlueprintCallable)
	FUpgradeInfo GetUpgradeInfo(const EUpgradeType Weapon) const;
	
	UFUNCTION(BlueprintCallable)
	void ApplyAllUpgradeFunctions(class APlayerCharacter* Player);

	
	void UseUpgradeFunction(const EUpgradeType Upgrade, class APlayerCharacter* Player);
	void UpgradePlayerStats(const EUpgradeType Upgrade, class APlayerCharacter* Player);
	void UpgradeGunStats(const EUpgradeType Upgrade, class APlayerCharacter* Player);
	
	UFUNCTION(BlueprintCallable)
	void StartDialogue(UAudioComponent* AudioComponent = nullptr);
	UFUNCTION()
	void PlayNextDialogue();
private:
	FTimerHandle TimerHandle;
	FString ConvertUpgradeTypeToString(const EUpgradeType Upgrade);

	void BuyWeapon(EUpgradeType Weapon);
};


