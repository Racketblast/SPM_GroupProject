#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Explosive.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AExplosive : public AActor
{
	GENERATED_BODY()

public:
	AExplosive();

protected:
	virtual void BeginPlay() override;
	virtual void Explode();

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* ExplosiveMesh;

	UPROPERTY(EditDefaultsOnly)
	class UProjectileMovementComponent* ProjectileComponent;

	UPROPERTY()
	AActor* InstigatorActor;  // Optional: track player that threw it

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	UParticleSystem* ExplosionEffectAsset;

	UPROPERTY(EditAnywhere, Category = "Explosion|Sound")
	USoundBase* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ExplosionDelay = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ExplosionRadius = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float WeaponDamage = 40.0f;

	float ExplosionPushForce = 600.0f;
	FTimerHandle ExplosionTimer;
	
};
