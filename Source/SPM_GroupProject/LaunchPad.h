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
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category="LaunchPad")
	bool bOverrideXY = false;

	UPROPERTY(EditAnywhere, Category="LaunchPad")
	bool bOverrideZ = false;

	UPROPERTY(EditAnywhere, Category="LaunchPad")
	FVector LaunchVector = {0,0,1000};
private:
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* LaunchPadMesh;
	
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* LaunchPadTriggerVolume;

	UFUNCTION()
	void LaunchPadTriggered(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
	//Not used but could be useful
	/*
	UFUNCTION()
	void LaunchPadEndTrigger(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex);
	*/

};
