#pragma once

#include "CoreMinimal.h"
#include "Gun.h"
#include "Components/PointLightComponent.h"
#include "Shotgun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AShotgun : public AGun
{
	GENERATED_BODY()

public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;
	void ApplyRecoilTranslation();
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	AActor* LastHitActor;

	UPROPERTY(BlueprintReadOnly)
	bool bEnemyHit = false;

	UFUNCTION()
	void EnemyHitFalse();
	virtual void BeginPlay() override;
	void BulletHoleDecal(const FHitResult& Hit);
	void ApplyBloodDecalTemp(const FHitResult& Hit);
	virtual void Tick(float DeltaTime) override;
protected:
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* BulletHitEffect;
	
	UPROPERTY(EditDefaultsOnly)
	int32 NumPellets = 10;

	UPROPERTY(EditDefaultsOnly)
	float SpreadAngle = 5.0f;

	UPROPERTY(EditDefaultsOnly)
	float Range = 1000.0f;
	

	float LastFireTime = 0.f;
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* BloodDecalMaterial;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* BulletDecalMaterial;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;
	bool bIsRecoveringFromRecoil = false;
	float RecoilRecoveryElapsed = 0.0f;
	float RecoilRecoveryDuration = 0.2f;

	FVector RecoilStartLocation;
	FVector RecoilTargetLocation;

	UPROPERTY()
	UPointLightComponent* MuzzleFlashLight;
};
