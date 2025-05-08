#pragma once

#include "CoreMinimal.h"
#include "Explosive.h"
#include "ImpactExplosive.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AImpactExplosive : public AExplosive
{
	GENERATED_BODY()

public:
	AImpactExplosive();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
