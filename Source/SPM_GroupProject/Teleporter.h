// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerUseInterface.h"
#include "GameFramework/Actor.h"
#include "Teleporter.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API ATeleporter : public AActor, public IPlayerUseInterface
{
	GENERATED_BODY()
	
public:
	ATeleporter();
	virtual void Use_Implementation(APlayerCharacter* Player) override;
	
	virtual void ShowInteractable_Implementation(bool bShow) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
	FName TargetLevelName = "Hub";
	
	UPROPERTY(EditDefaultsOnly, Category="Teleport")
	USoundBase* CantTeleportSound;

	UPROPERTY(EditDefaultsOnly, Category="Teleport")
	class UNiagaraComponent* TeleportSkyBeam;

	UPROPERTY(EditDefaultsOnly, Category="Teleport")
	class UNiagaraComponent* TeleportCircles;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void ChangeTexture();

	UFUNCTION(BlueprintCallable, Category="Teleport")
	void Teleport();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* CubeMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* TeleportTriggerVolume;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Teleport")
	TSubclassOf<UUserWidget> TeleportWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	class UMaterialInterface* WaveMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* GracePeriodMaterial;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:
	UPROPERTY()
	class UPlayerGameInstance* CachedGameInstance;
	bool bOldWaveValue;

	UFUNCTION()
	void ChangeLevel();
};
