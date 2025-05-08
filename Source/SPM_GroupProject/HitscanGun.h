#pragma once

#include "CoreMinimal.h"
#include "Gun.h"
#include "HitscanGun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AHitscanGun : public AGun
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;

	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	AActor* LastHitActor;

	UPROPERTY(BlueprintReadOnly)
	bool bEnemyHit = false;

	UFUNCTION()
	void EnemyHitFalse();
	
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* BulletHitEffect;

protected:
	UPROPERTY(EditDefaultsOnly)
	float Range = 2000.0f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* FireSound;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	UAudioComponent* FireAudioComponent;

	float LastFireTime = 0.f;
};

