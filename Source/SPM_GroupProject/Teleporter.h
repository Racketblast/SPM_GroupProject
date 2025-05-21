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
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void ChangeTexture();

	UFUNCTION()
	void Teleport();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* CubeMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	class UMaterialInterface* WaveMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* GracePeriodMaterial;
	
private:
	UPROPERTY()
	class UPlayerGameInstance* CachedGameInstance;
	bool bOldWaveValue;

	UFUNCTION()
	void ChangeLevel();
};
