// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollectableBox.h"
#include "AmmoBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AAmmoBox : public ACollectableBox
{
	GENERATED_BODY()
public:	
	UPROPERTY(EditAnywhere, Category="Ammo")
	int32 AmmoAmount = 30;
private:
	virtual void CollectableBoxTriggeredFunction(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
