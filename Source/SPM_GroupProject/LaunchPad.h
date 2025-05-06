// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaunchPad.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ALaunchPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaunchPad();

protected:
	UPROPERTY(EditAnywhere, Category="LaunchPad")
	bool bOverrideXY = false;

	UPROPERTY(EditAnywhere, Category="LaunchPad")
	bool bOverrideZ = false;

	UPROPERTY(EditAnywhere, Category="LaunchPad")
	FVector LaunchVector = {0,0,1000};
	
	UPROPERTY(EditDefaultsOnly, Category="LaunchPad")
	class USoundBase* LaunchSound;
private:
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* LaunchPadMesh;
	
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* LaunchPadTriggerVolume;

	UFUNCTION()
	void LaunchPadTriggered(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
