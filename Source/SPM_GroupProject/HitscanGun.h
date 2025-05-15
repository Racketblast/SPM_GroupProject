#pragma once

#include "PlayerCharacter.h"
#include "CoreMinimal.h"
#include "Gun.h"
#include "HitscanGun.generated.h"

// No bEnemyHit, no EnemyHitFalse!

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

	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* BulletHitEffect;
	virtual void Tick(float DeltaTime) override;
	void ApplyRecoilTranslation();
	
protected:
	UPROPERTY(EditDefaultsOnly)
	float Range = 2000.0f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;


	float LastFireTime = 0.f;
	// Variable to store the original location of the ArmsRoot before recoil
	// Flag to ensure recoil is only applied once per shot

};

