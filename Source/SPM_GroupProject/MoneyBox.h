// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoneyBox.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AMoneyBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoneyBox();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, Category="Money")
	int32 MoneyAmount = 20;
	
private:
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* MoneyMesh;
	
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* MoneyTriggerVolume;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* MoneyPickUpSound;
	
	UFUNCTION()
	void MoneyBoxTriggered(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,
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

	UPROPERTY(EditAnywhere, Category = "Self Destruct")
	float LifeTime = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Self Destruct")
	bool bShouldDestroy = true;
	
	FTimerHandle SelfDestructTimer;
	
	void SelfDestruct();
};
