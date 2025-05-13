// Gun.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AGun : public AActor
{
	GENERATED_BODY()

public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) PURE_VIRTUAL(AGun::Fire, );
	virtual void Reload();
	void SetOwnerCharacter(class APlayerCharacter* NewOwner);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun")
	bool bHasInfiniteReloads = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TotalAmmo = 90;
	AGun();  // 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WeaponDamage = 20;
	UPROPERTY(BlueprintReadOnly)
	float BaseWeaponDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RoundsPerSecond = 1;
	UPROPERTY(BlueprintReadOnly)
	float BaseRoundsPerSecond ;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RealoadSpeed = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* CurrentSkinMat;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* MuzzlePoint;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* WeaponSkeletalMesh;
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* MuzzleFlash;
	USceneComponent* GetMuzzlePoint() const { return MuzzlePoint; }
	//Upgrade variables
	UPROPERTY()
	bool bHasAppliedUpgrades = false;
	bool bIsUpgraded = false;
	void CheckForUpgrades();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	APlayerCharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere)
	int32 MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CurrentAmmo = MaxAmmo;
	
	bool bIsReloading = false;

	FTimerHandle ReloadTimerHandle;

	UFUNCTION()
	void FinishReload();
	// Sound asset for reload
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ReloadSound;


	UPROPERTY(VisibleAnywhere, Category = "Sound")
	UAudioComponent* ReloadAudioComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase*FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase*MagEmptySound;
	
	UPROPERTY(VisibleAnywhere, Category = "Sound")
	UAudioComponent* FireAudioComponent;

	// Audio component for reload sound
	UPROPERTY(VisibleAnywhere, Category = "Sound")
	UAudioComponent* MagEmptyAudioComponent;
	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilPitchMin = -3.0f;  // Upward

	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilPitchMax = -1.0f;  // Upward


	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilYawMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilYawMax = -0.5f;





};

