#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Explosive.h"
#include "Gun.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class SPM_GROUPPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	UFUNCTION(BlueprintCallable)
	AGun* GetCurrentGun() const;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(BlueprintReadOnly)
	bool bEnemyHit = false;

	UFUNCTION(BlueprintCallable)
	void EnemyHitFalse();


	UPROPERTY(EditAnywhere)
	USceneComponent* Hand;

	UPROPERTY(EditAnywhere)
	int32 UseDistance = 300;

	UPROPERTY(BlueprintReadWrite)
	int32 PlayerHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 PlayerMaxHealth = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 BasePlayerMaxHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 PickedUpMoney;

	UPROPERTY(BlueprintReadOnly)
	AGun* CurrentGun;

	UFUNCTION(BlueprintCallable)
	void SelectWeapon(FName Weapon);

	UFUNCTION(BlueprintCallable)
	void HealPlayer(int32 HealAmount);

	UFUNCTION(BlueprintCallable)
	AGun* GetWeaponInstance(const FName WeaponName) const;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	UPROPERTY(VisibleAnywhere, Category = "Sway")
	USceneComponent* ArmsRoot;

	UPROPERTY(BlueprintReadOnly)
	bool bCanSwitchWeapons = true;
protected:
	UPROPERTY(EditAnywhere)
	class UCameraComponent* PlayerCamera;

	void MoveForward(float InputVector);
	void MoveRight(float InputVector);
	void Yaw(float InputVector);
	void Pitch(float InputVector);
	void Shoot();
	void ThrowGrenade();
	void Use();
	void Reload();
	void SelectWeapon1();
	void SelectWeapon2();
	void SelectWeapon3();
	void SelectWeapon4();
	void SelectWeapon5();
	void StartShooting();
	void StopShooting();
	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;
	void AirDash();
	void UpdateFirstPersonMeshSway(float DeltaTime);


	bool Weapon1Equipped = false;
	bool Weapon2Equipped = false;
	bool Weapon3Equipped = false;
	bool Weapon4Equipped = false;
	bool Weapon5Equipped = false;
	
	UPROPERTY(BlueprintReadOnly)
	int32 GrenadeNum = 5;

private:
	UPROPERTY(Blueprintable)
	bool bIsDead = false;

	FName WeaponName1 = "Pistol";
	FName WeaponName2 = "Rifle";
	FName WeaponName3 = "Shotgun";
	FName WeaponName4 = "Rocketlauncher";
	FName WeaponName5 = "DoomsdayGun";

	UPROPERTY(EditAnywhere)
	TSubclassOf<AGun> GWeapon1;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGun> GWeapon2;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGun> GWeapon3;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGun> GWeapon4;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGun> GWeapon5;

	UPROPERTY()
	AGun* Weapon1Instance;
	UPROPERTY()
	AGun* Weapon2Instance;
	UPROPERTY()
	AGun* Weapon3Instance;
	UPROPERTY()
	AGun* Weapon4Instance;
	UPROPERTY()
	AGun* Weapon5Instance;

	bool bIsShooting = false;
	bool bHasDashed = false;
	float DashStrength = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	TSubclassOf<AExplosive> GrenadeClass;

	class UAIPerceptionStimuliSourceComponent* StimulusSource;
	void SetupStimulusSource();
	FTimerHandle EnemyHitResetTimerHandle;
	UPROPERTY(EditAnywhere, Category = "Sway")
	float SwayAmount = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Sway")
	float SwaySmoothing = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Sway")
	float MaxSwayAngle = 10.0f;

	FRotator TargetSwayRotation;
	FRotator CurrentSwayRotation;
	
	float CachedYawInput = 0.0f;
	float CachedPitchInput = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* DamageSound;

};
