// Gun.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AGun : public AActor
{
	GENERATED_BODY()

public:
	virtual void Fire(FVector FireLocation, FRotator FireRotation) PURE_VIRTUAL(AGun::Fire, );
	virtual void Reload();
	void SetOwnerCharacter(class APlayerCharacter* NewOwner);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun")
	bool bHasInfiniteReloads = false;

protected:
	UPROPERTY()
	APlayerCharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere)
	int32 MaxAmmo ;

	UPROPERTY(EditAnywhere)
	int32 CurrentAmmo = MaxAmmo;

	UPROPERTY(EditDefaultsOnly)
	int32 ExtraMags = 2;


};

