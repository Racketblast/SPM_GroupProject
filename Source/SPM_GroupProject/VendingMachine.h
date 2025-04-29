// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VendingMachine.generated.h"

UENUM(BlueprintType)
enum class EVendingMachineSpewOut : uint8
{
	None        UMETA(DisplayName = "None"),
	Health        UMETA(DisplayName = "Health"),
	Ammo        UMETA(DisplayName = "Ammo"),
	// Add more as needed
};
UCLASS()
class SPM_GROUPPROJECT_API AVendingMachine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVendingMachine();
	
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
