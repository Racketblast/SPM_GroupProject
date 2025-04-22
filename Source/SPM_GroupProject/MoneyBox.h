// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollectableBox.h"
#include "MoneyBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AMoneyBox : public ACollectableBox
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:		
	UPROPERTY(EditAnywhere, Category="Money")
	int32 MoneyAmount = 20;
	
private:
	virtual void CollectableBoxTriggeredFunction(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	UPROPERTY(EditAnywhere, Category = "Self Destruct")
	float LifeTime = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Self Destruct")
	bool bShouldDestroy = true;
	
	FTimerHandle SelfDestructTimer;
	
	void SelfDestruct();
};
