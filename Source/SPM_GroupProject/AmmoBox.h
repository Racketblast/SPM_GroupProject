// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AAmmoBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmmoBox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, Category="Ammo")
	int32 AmmoAmount = 1;
private:
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* AmmoMesh;
	
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* AmmoTriggerVolume;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* AmmoPickUpSound;
	
	UFUNCTION()
	void AmmoBoxTriggered(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(EditAnywhere)
	float DistFloat = -1;
	
	FVector StartVector;
	
	UPROPERTY(EditDefaultsOnly, Category = "moving")
	FVector PlatformVelocity;
	
	UPROPERTY(EditDefaultsOnly, Category = "rotation")
	FRotator RotationVelocity;

	void MoveBox(float DeltaTime);
	void RotateBox(float DeltaTime);
	
	bool ShouldBoxReturn() const;
	float GetDistanceMoved() const;
};
