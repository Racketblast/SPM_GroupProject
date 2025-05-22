// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "CrazyBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ACrazyBox : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrazyBox();
	virtual void Use_Implementation(APlayerCharacter* Player) override;
	
	virtual void ShowInteractable_Implementation(bool bShow) override;

protected:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* CubeMeshComponent;

	bool bCrazy = false;
};
