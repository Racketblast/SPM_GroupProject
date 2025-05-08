#pragma once

#include "CoreMinimal.h"
#include "Gun.h"
#include "Shotgun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AShotgun : public AGun
{
	GENERATED_BODY()

public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;

	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	AActor* LastHitActor;

	UPROPERTY(BlueprintReadOnly)
	bool bEnemyHit = false;

	UFUNCTION()
	void EnemyHitFalse();
	virtual void BeginPlay() override;
protected:
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* BulletHitEffect;
	
	UPROPERTY(EditDefaultsOnly)
	int32 NumPellets = 10;

	UPROPERTY(EditDefaultsOnly)
	float SpreadAngle = 5.0f;

	UPROPERTY(EditDefaultsOnly)
	float Range = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* FireSound;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	UAudioComponent* FireAudioComponent;

	float LastFireTime = 0.f;
};
