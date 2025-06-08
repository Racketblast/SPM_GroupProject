// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Character.h"

#include "AI_Main.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAIDeathEvent);

UCLASS()
class SPM_GROUPPROJECT_API AAI_Main : public ACharacter
{
	GENERATED_BODY()

public:
	AAI_Main();

	UBehaviorTree* GetBehaviorTree() const;

	UPROPERTY(BlueprintReadWrite)
	int32 AIHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAIHealth = 100;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float AIDamage = 20;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<TSubclassOf<class ACollectableBox>>AIDrop;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(BlueprintReadWrite)
	bool bIsAttacking = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	class UAudioComponent* AudioComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UNiagaraSystem* DamageEffect;
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* DeathEffect;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAIDeathEvent, AAI_Main*, DeadEnemy);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FAIDeathEvent OnEnemyDied;
public:	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsDead = false;

private:
	
	FVector LastKnownLocation;
	float TimeSinceLastMovement = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport Check")
	float StuckCheckInterval = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport Check")
	float MinMoveDistance = 10.0f;

	bool IsOutsideNavMesh() const;
};
