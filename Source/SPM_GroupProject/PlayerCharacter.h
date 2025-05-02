// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "HitscanGun.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 PlayerHealth;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 PlayerMaxHealth = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 PickedUpMoney;
	
	UPROPERTY(BlueprintReadOnly)
	class AGun* CurrentGun;

	UFUNCTION(BlueprintCallable)
	void SelectWeapon(FName Weapon);
	
	UFUNCTION(BlueprintCallable)
	void HealPlayer(int32 HealAmount);

	UFUNCTION(BlueprintCallable)
	AGun* GetWeaponInstance(const FName WeaponName) const;
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
	void SelectWeapon3();
	void StartShooting();
	void StopShooting();
	virtual void Jump() override; // La till detta för challenge systemet 

	bool Weapon1Equipped = false;
	bool Weapon2Equipped = false;
	bool Weapon3Equipped = false;
	
private:
	
	FName WeaponName1 = "Pistol";
	FName WeaponName2 = "Rifle";
	FName WeaponName3 = "Laser"; 
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGun> GWeapon1;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGun> GWeapon2;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGun> GWeapon3; // Set this to HitscanGun subclass in the editor
	UPROPERTY()
	AGun* Weapon1Instance;
	UPROPERTY()
	AGun* Weapon2Instance;
	UPROPERTY()
	AGun* Weapon3Instance;
	bool bIsShooting = false; // True when the player is holding the shoot button
	UPROPERTY(EditDefaultsOnly, Category="Transition")
	class ULevelSequence* FadeInTransition;
	UPROPERTY(EditDefaultsOnly, Category="Transition")
	class USoundBase* TeleportInSound;
	


	class UAIPerceptionStimuliSourceComponent* StimulusSource;
	void SetupStimulusSource();


};