// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuyBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ABuyBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuyBox();

	UPROPERTY(EditAnywhere)
	FName TargetUpgradeName;

	UPROPERTY(EditAnywhere)
	int32 TargetUpgradeCost;
protected:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* CubeMeshComponent;
};
