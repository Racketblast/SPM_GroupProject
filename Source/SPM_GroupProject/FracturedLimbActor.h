// FracturedLimbActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "Physics/Experimental/ChaosInterfaceWrapper.h"
#include "FracturedLimbActor.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AFracturedLimbActor : public AActor
{
	GENERATED_BODY()
    
public:    
	AFracturedLimbActor();
	void HandleRagdollBoneHit(FName BoneName, FVector HitLocation, float DamageAmount);
protected:
	virtual void BeginPlay() override;

public:    
	// Geometry Collection Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fracture")
	UGeometryCollectionComponent* GeometryCollectionComp;

	// The fracture asset to use
	UPROPERTY(EditDefaultsOnly, Category = "Fracture")
	UGeometryCollection* FractureAsset;
	UPROPERTY()
	TMap<FName, AFracturedLimbActor*> AttachedFracturedLimbs;
};
