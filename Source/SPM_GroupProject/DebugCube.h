// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "DebugCube.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ADebugCube : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADebugCube();
	virtual void Use_Implementation(APlayerCharacter* Player) override;
	
	virtual void ShowInteractable_Implementation(bool bShow) override;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bLevel = false;
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bHealth = false;
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bMoney = false;
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bUnlockAllWeapons = false;
	void DoAllFunctions();
protected:
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	class USoundBase* UseSound;
	
	void EnableAllLevels();
	void GivePlayerMaxHealth();
	void GivePlayerMaxMoney();
	void GiveAllWeapons();
};
