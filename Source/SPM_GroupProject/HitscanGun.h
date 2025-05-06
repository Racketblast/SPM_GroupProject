#pragma once

#include "CoreMinimal.h"
#include "Gun.h"
#include "HitscanGun.generated.h"


UCLASS()
class SPM_GROUPPROJECT_API AHitscanGun : public AGun
{
	GENERATED_BODY()
	virtual void BeginPlay() override;
public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	AActor* LastHitActor;
protected:
	UPROPERTY(EditDefaultsOnly)
	float Range = 2000.0f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;
	
	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilPitchMin = -3.0f;  // Upward

	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilPitchMax = -1.0f;  // Upward


	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilYawMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilYawMax = -0.5f;
	// Header file

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* FireSound;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	UAudioComponent* FireAudioComponent;
	float LastFireTime = 0.f;




};

