// ProjectileGun.h
#pragma once

#include "Gun.h"
#include "projectile.h"
#include "ProjectileGun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AProjectileGun : public AGun
{
	GENERATED_BODY()

public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;
	void SetProjectileClass(TSubclassOf<AProjectile> NewClass)
	{
		ProjectileClass = NewClass;
	}

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AProjectile> ProjectileClass;
};
