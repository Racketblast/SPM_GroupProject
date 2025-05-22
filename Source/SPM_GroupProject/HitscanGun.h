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
	void ApplyBloodDecal(const FHitResult& Hit);
	void BulletHoleDecal(const FHitResult& Hit);
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
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UMaterialInterface* BloodDecalMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UMaterialInterface* BulletDecalMaterial;

};

