#pragma once

#include "CoreMinimal.h"
#include "Gun.h"
#include "HitscanGun.generated.h"


UCLASS()
class SPM_GROUPPROJECT_API AHitscanGun : public AGun
{
	GENERATED_BODY()
	
public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	float Range = 2000.0f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly)
	float Damage = 25.0f;
};

