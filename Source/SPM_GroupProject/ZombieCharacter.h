#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

class AFracturedLimbActor;

UCLASS()
class SPM_GROUPPROJECT_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AZombieCharacter();

	// Call this on bone damage
	void HandleBoneHit(FName BoneName, FVector HitLocation, float DamageAmount);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.f;

	float CurrentHealth;

	// Per-limb health
	UPROPERTY(EditDefaultsOnly, Category = "Limb Health")
	TMap<FName, float> LimbHealth;

	// Starting health per limb
	UPROPERTY(EditDefaultsOnly, Category = "Limb Health")
	float DefaultLimbHealth = 150.f;

	// Fracture setup
	UPROPERTY(EditDefaultsOnly, Category = "Fracture")
	TSubclassOf<AFracturedLimbActor> FracturedLimbClass;

	// Keeps track of destroyed bones
	UPROPERTY()
	TSet<FName> DestroyedBones;

	// Initializes limb health map
	void InitializeLimbHealth();
};
