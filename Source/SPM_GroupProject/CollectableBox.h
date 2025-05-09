// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollectableBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ACollectableBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACollectableBox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* PhysicsRoot;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* CollectableMesh;
	
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* CollectableTriggerVolume;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* CollectablePickUpSound;

	UPROPERTY(EditAnywhere)
	float DistFloat = 10;
	
	FVector StartVector;
	
	UPROPERTY(EditDefaultsOnly, Category = "moving")
	FVector PlatformVelocity = {0,0,5};
	
	UPROPERTY(EditDefaultsOnly, Category = "rotation")
	FRotator RotationVelocity = {0,90,0};

	void MoveBox(float DeltaTime);
	void RotateBox(float DeltaTime);
	
	bool ShouldBoxReturn() const;
	float GetDistanceMoved() const;

	UFUNCTION()
	void CollectableBoxTriggered(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void CollectableBoxTriggeredFunction(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
