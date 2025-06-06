#pragma once

#include "CoreMinimal.h"
#include "Gun.h"
#include "HitscanGun.h"
#include "Components/PointLightComponent.h"
#include "Shotgun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AShotgun : public AHitscanGun
{
	GENERATED_BODY()

public:
	AShotgun();
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;


	UPROPERTY(BlueprintReadOnly)
	bool bEnemyHit = false;

	UFUNCTION()
	void EnemyHitFalse();
protected:

	
	UPROPERTY(EditDefaultsOnly)
	int32 NumPellets = 10;

	UPROPERTY(EditDefaultsOnly)
	float SpreadAngle = 5.0f;

	//For pump sound
	FTimerHandle PumpTimer;
	UFUNCTION()
	void PumpSoundDelay();
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* PumpSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	UAudioComponent* PumpAudioComponent;


	


};


