// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "VendingMachine.generated.h"

UENUM(BlueprintType)
enum class EVendingMachineSpewOut : uint8
{
	None        UMETA(DisplayName = "None"),
	Health        UMETA(DisplayName = "Health"),
	AmmoRifle	UMETA(DisplayName = "Ammo Rifle"),
	AmmoShotgun	UMETA(DisplayName = "Ammo Shotgun"),
	AmmoRocketLauncher	UMETA(DisplayName = "Ammo RocketLauncher"),
	// Add more as needed
};
UCLASS()
class SPM_GROUPPROJECT_API AVendingMachine : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVendingMachine();
	virtual void Use_Implementation(APlayerCharacter* Player) override;
	
	virtual void ShowInteractable_Implementation(bool bShow) override;
	
	void UseVendingMachine();
protected:
	UPROPERTY(EditAnywhere)
	EVendingMachineSpewOut SpewOut = EVendingMachineSpewOut::None;

	UPROPERTY(EditAnywhere)
	int32 SpewOutCost;

	UPROPERTY(EditAnywhere)
	int32 SpewOutAmount;
	
	UPROPERTY(EditDefaultsOnly)
	class USoundBase* CanBuySound;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* CantBuySound;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* StaticMeshComponent;
	
	bool IfPlayerHasEnoughMoney(class APlayerCharacter* Player, class UPlayerGameInstance* GI);

};
