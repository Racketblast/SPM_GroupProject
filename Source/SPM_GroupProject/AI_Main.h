// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Character.h"
#include "AI_Main.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API AAI_Main : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAI_Main();

	UBehaviorTree* GetBehaviorTree() const;

	UPROPERTY(BlueprintReadWrite)
	int32 AIHealth;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 MaxAIHealth = 100;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(BlueprintReadWrite)
	bool bIsAttacking = false;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	class UAudioComponent* AudioComponent;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	bool bIsDead = false;
};
