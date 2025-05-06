// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "StoreBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AStoreBox : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStoreBox();
	virtual void Use_Implementation(APlayerCharacter* Player) override;

	void OpenStoreMenu();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> BuyBoxWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> HudWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	class USoundBase* OpenBuyMenuSound;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* StaticMeshComponent;

};
