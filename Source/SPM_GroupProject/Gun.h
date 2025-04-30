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

	//Right now these do nothing, but it should be used for all weapons
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WeaponDamage = 20;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RoundsPerSecond = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RealoadSpeed = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* CurrentSkinMat;

	//Upgrade variables
	UPROPERTY()
	bool bHasAppliedUpgrades = false;
	bool bIsUpgraded = false;
	void CheckForUpgrades();
protected:
	UPROPERTY()
	APlayerCharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere)
	int32 MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CurrentAmmo = MaxAmmo;
	


};

