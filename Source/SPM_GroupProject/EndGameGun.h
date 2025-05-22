// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gun.h"
#include "EndGameGun.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GROUPPROJECT_API AEndGameGun : public AGun
{
	GENERATED_BODY()
	virtual void Fire(FVector FireLocation, FRotator FireRotation) override;
	void ApplyRecoilTranslation();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AEndGameExplosive> ProjectileClass;
	
};
