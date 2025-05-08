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

	// Explosive.h

	UPROPERTY()
	AActor* InstigatorActor;  // Store the player who threw the explosive


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	UParticleSystem* ExplosionEffectAsset;  // Reference to the particle system asset

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ExplosionDelay = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ExplosionRadius = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float WeaponDamage = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Explosion|Sound")
	USoundBase* ExplosionSound;


	FTimerHandle ExplosionTimer;
};
