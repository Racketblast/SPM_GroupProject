#pragma once

#include "CoreMinimal.h"
#include "HitscanGun.h"
#include "Shotgun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AShotgun : public AHitscanGun
{
	GENERATED_BODY()

public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Shotgun")
	int32 NumPellets = 9;

	UPROPERTY(EditDefaultsOnly, Category = "Shotgun")
	float SpreadAngle = 6.0f; // Degrees of spread per pellet
};

