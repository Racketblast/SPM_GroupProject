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
	virtual void Tick(float DeltaTime) override;
	void SetProjectileClass(TSubclassOf<AExplosive> NewClass)
	{
		ProjectileClass = NewClass;
	}
void BeginPlay();
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AExplosive> ProjectileClass;

private:
	bool bCanFire = true;
	FTimerHandle FireRateTimerHandle;

	void ResetFire();
	// ProjectileGun.h

void ApplyRecoilTranslation();
	// For interpolation
	bool bIsRecoveringFromRecoil = false;
	float RecoilRecoveryElapsed = 0.0f;
	float RecoilRecoveryDuration = 0.3f;

	FVector RecoilStartLocation;
	FVector RecoilTargetLocation;
	UPROPERTY()
	UPointLightComponent* MuzzleFlashLight;
};
