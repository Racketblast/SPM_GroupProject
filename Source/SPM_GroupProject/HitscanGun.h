#pragma once

#include "PlayerCharacter.h"
#include "CoreMinimal.h"
#include "Gun.h"
#include "Components/PointLightComponent.h"
#include "HitscanGun.generated.h"



UCLASS()
class SPM_GROUPPROJECT_API AHitscanGun : public AGun
{
	GENERATED_BODY()

protected:

	
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;
public:
	
	
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	AActor* LastHitActor;

	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* BulletHitEffect;
	

	
protected:
	UPROPERTY(EditDefaultsOnly)
	float Range = 2000.0f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;


	float LastFireTime = 0.f;





	UPROPERTY()
	UPointLightComponent* MuzzleFlashLight;

};

