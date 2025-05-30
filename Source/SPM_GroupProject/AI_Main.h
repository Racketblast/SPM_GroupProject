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
    /* ---------------- CONSTRUCTION ---------------- */
    /// Sets default property values and creates components that must exist
    /// before BeginPlay (e.g. audio component).
    AAI_Main();

    /* ---------------- GETTERS --------------------- */
    /// Needed by AI_Controller::RunBehaviorTree().
    UBehaviorTree* GetBehaviorTree() const;

    /* ---------------- Gameplay stats -------------- */
    UPROPERTY(BlueprintReadWrite)                     // Current health – replicated to BP
    int32 AIHealth = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   // Max health, tweakable in editor
    int32 MaxAIHealth = 100;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   // Damage dealt per projectile/melee
    float AIDamage = 20.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   // Loot crate to spawn on death
    TSubclassOf<class ACollectableBox> AIDrop;

    /* ------------- AActor overrides --------------- */
    virtual float TakeDamage(float DamageAmount,
                             struct FDamageEvent const& DamageEvent,
                             class AController* EventInstigator,
                             AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;                // Init runtime state

    /* ---------------- AI data --------------------- */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
    UBehaviorTree* BehaviorTree = nullptr;            // BT asset assigned in editor

    UPROPERTY(BlueprintReadWrite)                     // Used by anim BP to gate attacks
    bool bIsAttacking = false;

    /* ---------------- SFX / VFX ------------------- */
    UPROPERTY(EditDefaultsOnly, Category="Sound")
    class UAudioComponent* AudioComponent = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="Effects")
    class UNiagaraSystem* DamageEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="Effects")
    UNiagaraSystem* DeathEffect = nullptr;

public:
    virtual void Tick(float DeltaTime) override;      // Per‑frame behavior
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    /* ------------- Internal state ----------------- */
    bool bIsDead = false;             // Guard to run death logic only once

    /* ------------- “Stuck” detection -------------- */
    FVector LastKnownLocation;        // Where we last confirmed movement
    float   TimeSinceLastMovement = 2.f;

    UPROPERTY(EditDefaultsOnly, Category="Teleport Check")
    float StuckCheckInterval = 5.f;   // Seconds with no movement before flag

    UPROPERTY(EditDefaultsOnly, Category="Teleport Check")
    float MinMoveDistance   = 10.f;   // Squared distance threshold (cm)

    bool IsOutsideNavMesh() const;    // Helper: true if pawn fell off navigation
};

