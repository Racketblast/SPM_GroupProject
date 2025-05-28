#pragma once

#include "Explosive.h"
#include "Gun.h"
#include "Components/PointLightComponent.h"
#include "ProjectileGun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AProjectileGun : public AGun
{
	GENERATED_BODY()

public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;
	void SetProjectileClass(TSubclassOf<AExplosive> NewClass)
	{
		ProjectileClass = NewClass;
	}
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AExplosive> ProjectileClass;

private:
	bool bCanFire = true;
	FTimerHandle FireRateTimerHandle;

	void ResetFire();
	
	UPROPERTY()
	UPointLightComponent* MuzzleFlashLight;
};
