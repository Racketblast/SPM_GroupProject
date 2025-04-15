// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere)
	USceneComponent* Hand;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	

	UPROPERTY(EditAnywhere)
	int32 UseDistance = 300;
protected:
	UPROPERTY(EditAnywhere)
	UCameraComponent* PlayerCamera;
	void MoveForward(float InputVector);
	void MoveRight(float InputVector);
	void Yaw(float InputVector);
	void Pitch(float InputVector);
	void Shoot();
	void Use();
	void Reload();
	void SelectWeapon1();
	void SelectWeapon2();
	bool Weapon1Equipped;
	bool Weapon2Equipped;
	
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentMaxAmmo;
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentAmmo;
	
private:
	FName WeaponName1 = "Pistol";
	FName WeaponName2 = "Rifle";
	int32 MaxAmmo1 = 9;
	int32 MaxAmmo2 = 30;
	int32 Ammo1 = MaxAmmo1;
	int32 Ammo2 = MaxAmmo2;
	
	
	AActor* TargetActor;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileActor;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> Weapon1;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> Weapon2;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GWeapon1;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GWeapon2;
};