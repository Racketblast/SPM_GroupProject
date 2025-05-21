// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "TeleportScreen.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ATeleportScreen : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	ATeleportScreen();
	virtual void Use_Implementation(APlayerCharacter* Player) override;
	
	virtual void ShowInteractable_Implementation(bool bShow) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> TeleportWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> HudWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	class USoundBase* OpenTeleportMenuSound;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* StaticMeshComponent;

};
