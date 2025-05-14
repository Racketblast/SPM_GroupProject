// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"

class UPlayerGameInstance;
enum class EUpgradeType : uint8;
enum class EUpgradeCategory : uint8;

#include "BuyBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ABuyBox : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuyBox();
	virtual void Use_Implementation(APlayerCharacter* Player) override;

	UPROPERTY(EditAnywhere, Category="Upgrade")
	EUpgradeType TargetUpgradeName;
	
	UPROPERTY(EditDefaultsOnly, Category="Upgrade")
	class USoundBase* BuySound;
	
	UPROPERTY(EditDefaultsOnly, Category="Upgrade")
	USoundBase* CantBuySound;
protected:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* CubeMeshComponent;
};
