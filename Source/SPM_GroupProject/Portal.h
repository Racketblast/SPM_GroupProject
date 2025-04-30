// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly)
	USceneComponent* PortalSceneComponent;
	
	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* PortalMeshComponent;
		
	UPROPERTY(EditAnywhere)
	class USceneCaptureComponent2D* PortalCaptureComponent;

	UPROPERTY(EditAnywhere)
	class USceneCaptureComponent2D* PortalNormalCaptureComponent;
	
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* PortalTriggerVolume;
	
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* PortalDistanceTriggerVolume;
	
	UPROPERTY(EditDefaultsOnly)
	class UAudioComponent* PortalSoundComponent;
	
	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* PortalMaterial;
	
	UPROPERTY()
	class UMaterialInstanceDynamic* PortalMaterialInstance;

	UPROPERTY(EditDefaultsOnly)
	class UArrowComponent* ForwardDirection;

	UPROPERTY()
	class UTextureRenderTarget2D* PortalRenderTarget;
	UPROPERTY()
	UTextureRenderTarget2D* PortalFarawayRenderTarget;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class APortal* LinkedPortal = this;
private:
	UPROPERTY()
	class APlayerCameraManager* PlayerCamera;
	
	UFUNCTION()
	void OnTriggerCloseBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnTriggerCloseEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void SetClipPlanes();
	void UpdatePortalCapture();
	void CheckViewportSize() const;

	UFUNCTION()
	void SetPortalMaterial();

	void ChangePortalMaterial(float DeltaTime);

	FVector UpdateVelocity(FVector Velocity);

	void ShouldTeleport();
	bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);
	void Teleport(AActor* OtherActor);

	//For warpDistance basically. Plus means that you can go through it and not teleport, if Minus means that you go through the portal further away
	//What it actually does is pushing the material backwards (negative value) or forwards (plus value)
	float TextureOffsetAmount = -17;
	
	//Teleport variables
	FVector LastPosition;
	bool bLastInFront;

	float TargetDeltaTime;
	float AccumulatedTime;
	bool bPlayerInRange = false;

	//For blending materials
	float DistanceBlendAlpha = 0.0f;
	float BlendSpeed = 4.0f;
	bool bBlendingToFaraway = true;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
