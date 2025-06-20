#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeformTest.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ADeformTest : public AActor
{
	GENERATED_BODY()

public:
	ADeformTest();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY()
	UMaterialInstanceDynamic* DynMaterial;

	// Call this to apply deformation at a world location
	UFUNCTION(BlueprintCallable)
	void ApplyDeformation(const FVector& WorldHitLocation);
};
