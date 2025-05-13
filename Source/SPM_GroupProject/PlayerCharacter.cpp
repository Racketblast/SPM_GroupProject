#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "Teleporter.h"
#include "Gun.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequencePlayer.h"
#include "ChallengeSubsystem.h"
#include "ArenaGameMode.h"
#include "Rifle.h"
#include "Shotgun.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Explosive.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;
	GetMesh()->SetupAttachment(PlayerCamera);

	SetupStimulusSource();
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	BasePlayerMaxHealth = PlayerMaxHealth;

	Weapon1Instance = GetWorld()->SpawnActor<AGun>(GWeapon1);
	Weapon1Instance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_rSocket"));
	Weapon1Instance->SetOwnerCharacter(this);
	Weapon1Instance->SetActorHiddenInGame(true);
	Weapon1Instance->SetActorEnableCollision(false);

	Weapon2Instance = GetWorld()->SpawnActor<AGun>(GWeapon2);
	Weapon2Instance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_rSocket"));
	Weapon2Instance->SetOwnerCharacter(this);
	Weapon2Instance->SetActorHiddenInGame(true);
	Weapon2Instance->SetActorEnableCollision(false);

	Weapon3Instance = GetWorld()->SpawnActor<AGun>(GWeapon3);
	Weapon3Instance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_rSocket"));
	Weapon3Instance->SetOwnerCharacter(this);
	Weapon3Instance->SetActorHiddenInGame(true);
	Weapon3Instance->SetActorEnableCollision(false);

	Weapon4Instance = GetWorld()->SpawnActor<AGun>(GWeapon4);
	Weapon4Instance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_rSocket"));
	Weapon4Instance->SetOwnerCharacter(this);
	Weapon4Instance->SetActorHiddenInGame(true);
	Weapon4Instance->SetActorEnableCollision(false);

	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
			// Pistol
			if (!GI->UpgradeMap.Contains(EUpgradeType::Pistol))
			{
				GI->UpgradeMap.Add(EUpgradeType::Pistol, GI->SetDefaultUpgradeInfo(EUpgradeType::Pistol));
				GI->SetCurrentWeapon(EUpgradeType::Pistol);
			}

			// Apply all upgrades
			GI->ApplyAllUpgradeFunctions(this);
			SelectWeapon(GI->GetCurrentWeaponName());
	}


	PlayerHealth = PlayerMaxHealth;

	if (AArenaGameMode* GameMode = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->FadeIn(this);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Weapon2Equipped && CurrentGun && CurrentGun->IsA<ARifle>() && bIsShooting)
	{
		Shoot();
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("AirDash", IE_Pressed, this, &APlayerCharacter::AirDash);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &APlayerCharacter::Shoot);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &APlayerCharacter::ThrowGrenade);
	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &APlayerCharacter::Use);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::Reload);

	PlayerInputComponent->BindAction("SelectWeapon1", IE_Pressed, this, &APlayerCharacter::SelectWeapon1);
	PlayerInputComponent->BindAction("SelectWeapon2", IE_Pressed, this, &APlayerCharacter::SelectWeapon2);
	PlayerInputComponent->BindAction("SelectWeapon3", IE_Pressed, this, &APlayerCharacter::SelectWeapon3);
	PlayerInputComponent->BindAction("SelectWeapon4", IE_Pressed, this, &APlayerCharacter::SelectWeapon4);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Yaw", this, &APlayerCharacter::Yaw);
	PlayerInputComponent->BindAxis("Pitch", this, &APlayerCharacter::Pitch);

	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &APlayerCharacter::StartShooting);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &APlayerCharacter::StopShooting);
}

void APlayerCharacter::StartShooting()
{
	if (Weapon2Equipped && CurrentGun && CurrentGun->IsA<ARifle>())
	{
		bIsShooting = true;
	}
}

void APlayerCharacter::StopShooting()
{
	bIsShooting = false;
}

AGun* APlayerCharacter::GetCurrentGun() const
{
	return CurrentGun;
}

void APlayerCharacter::ThrowGrenade()
{
	if (!GrenadeClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Grenade class not set!"));
		return;
	}
	if (GrenadeNum > 0){
		FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 50);
		FRotator SpawnRotation = PlayerCamera->GetComponentRotation();

		AExplosive* SpawnedGrenade = GetWorld()->SpawnActor<AExplosive>(GrenadeClass, SpawnLocation, SpawnRotation);
		SpawnedGrenade->SetInstigator(this);
		GrenadeNum--;
	}
}

void APlayerCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void APlayerCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void APlayerCharacter::Yaw(float Value)
{
	AddControllerYawInput(Value);
}

void APlayerCharacter::Pitch(float Value)
{
	AddControllerPitchInput(Value);
}

void APlayerCharacter::Jump()
{
	if (UChallengeSubsystem* ChallengeSub = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
	{
		ChallengeSub->NotifyPlayerJumped();
	}

	Super::Jump();
}

void APlayerCharacter::SetupStimulusSource()
{
	StimulusSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimulus"));
	if (StimulusSource)
	{
		StimulusSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
		StimulusSource->RegisterWithPerceptionSystem();
	}
}
void APlayerCharacter::EnemyHitFalse()
{
	bEnemyHit = false;
	UE_LOG(LogTemp, Error, TEXT("hit false (player)"));
}


void APlayerCharacter::Shoot()
{
	if (!CurrentGun) return;

	USceneComponent* Muzzle = CurrentGun->GetMuzzlePoint();
	if (!Muzzle) return;

	if (CurrentGun == Weapon4Instance)
	{
		CurrentGun->Fire(Muzzle->GetComponentLocation(), PlayerCamera->GetComponentRotation());
	}
	else
	{
		CurrentGun->Fire(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation());
	}

	if (UChallengeSubsystem* ChallengeSubsystem = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
	{
		if (CurrentGun == Weapon1Instance) ChallengeSubsystem->NotifyWeaponFired(WeaponName1);
		else if (CurrentGun == Weapon2Instance) ChallengeSubsystem->NotifyWeaponFired(WeaponName2);
		else if (CurrentGun == Weapon3Instance) ChallengeSubsystem->NotifyWeaponFired(WeaponName3);
		else if (CurrentGun == Weapon4Instance) ChallengeSubsystem->NotifyWeaponFired(WeaponName4);
	}
}

void APlayerCharacter::Reload()
{
	if (CurrentGun)
	{
		CurrentGun->Reload();
	}
}

void APlayerCharacter::SelectWeapon(FName Weapon)
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (Weapon == WeaponName1) SelectWeapon1();
		else if (Weapon == WeaponName2) SelectWeapon2();
		else if (Weapon == WeaponName3) SelectWeapon3();
		else if (Weapon == WeaponName4) SelectWeapon4();
	}
}

void APlayerCharacter::SelectWeapon1()
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon1Equipped && GI->HasBought(WeaponName1))
		{
			GI->SetCurrentWeapon(WeaponName1);
			Weapon1Equipped = true;
			Weapon2Equipped = Weapon3Equipped = Weapon4Equipped = false;

			if (Weapon1Instance)
			{
				Weapon1Instance->SetActorHiddenInGame(false);
				Weapon1Instance->SetActorEnableCollision(true);
				CurrentGun = Weapon1Instance;
				CurrentGun->CheckForUpgrades();
			}

			if (Weapon2Instance) Weapon2Instance->SetActorHiddenInGame(true);
			if (Weapon3Instance) Weapon3Instance->SetActorHiddenInGame(true);
			if (Weapon4Instance) Weapon4Instance->SetActorHiddenInGame(true);
		}
	}
}

void APlayerCharacter::SelectWeapon2()
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon2Equipped && GI->HasBought(WeaponName2))
		{
			GI->SetCurrentWeapon(WeaponName2);
			Weapon2Equipped = true;
			Weapon1Equipped = Weapon3Equipped = Weapon4Equipped = false;

			if (Weapon2Instance)
			{
				Weapon2Instance->SetActorHiddenInGame(false);
				Weapon2Instance->SetActorEnableCollision(true);
				CurrentGun = Weapon2Instance;
				CurrentGun->CheckForUpgrades();
			}

			if (Weapon1Instance) Weapon1Instance->SetActorHiddenInGame(true);
			if (Weapon3Instance) Weapon3Instance->SetActorHiddenInGame(true);
			if (Weapon4Instance) Weapon4Instance->SetActorHiddenInGame(true);
		}
	}
}

void APlayerCharacter::SelectWeapon3()
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon3Equipped && GI->HasBought(WeaponName3))
		{
			GI->SetCurrentWeapon(WeaponName3);
			Weapon3Equipped = true;
			Weapon1Equipped = Weapon2Equipped = Weapon4Equipped = false;

			if (Weapon3Instance)
			{
				Weapon3Instance->SetActorHiddenInGame(false);
				Weapon3Instance->SetActorEnableCollision(true);
				CurrentGun = Weapon3Instance;
				CurrentGun->CheckForUpgrades();
			}

			if (Weapon1Instance) Weapon1Instance->SetActorHiddenInGame(true);
			if (Weapon2Instance) Weapon2Instance->SetActorHiddenInGame(true);
			if (Weapon4Instance) Weapon4Instance->SetActorHiddenInGame(true);
		}
	}
}

void APlayerCharacter::SelectWeapon4()
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon4Equipped && GI->HasBought(WeaponName4))
		{
			GI->SetCurrentWeapon(WeaponName4);
			Weapon4Equipped = true;
			Weapon1Equipped = Weapon2Equipped = Weapon3Equipped = false;

			if (Weapon4Instance)
			{
				Weapon4Instance->SetActorHiddenInGame(false);
				Weapon4Instance->SetActorEnableCollision(true);
				CurrentGun = Weapon4Instance;
				CurrentGun->CheckForUpgrades();
			}

			if (Weapon1Instance) Weapon1Instance->SetActorHiddenInGame(true);
			if (Weapon2Instance) Weapon2Instance->SetActorHiddenInGame(true);
			if (Weapon3Instance) Weapon3Instance->SetActorHiddenInGame(true);
		}
	}
}

void APlayerCharacter::Use()
{
	FVector Start = PlayerCamera->GetComponentLocation();
	FVector End = Start + PlayerCamera->GetForwardVector() * UseDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		if (AActor* Actor = HitResult.GetActor())
		{
			if (Actor->GetClass()->ImplementsInterface(UPlayerUseInterface::StaticClass()))
			{
				IPlayerUseInterface::Execute_Use(Actor, this);
			}
		}
	}
}

void APlayerCharacter::HealPlayer(int32 HealAmount)
{
	if (!bIsDead)
	{
		PlayerHealth = FMath::Clamp(PlayerHealth + HealAmount, 0, PlayerMaxHealth);
	}
}

AGun* APlayerCharacter::GetWeaponInstance(const FName WeaponName) const
{
	if (WeaponName == WeaponName1) return Weapon1Instance;
	if (WeaponName == WeaponName2) return Weapon2Instance;
	if (WeaponName == WeaponName3) return Weapon3Instance;
	if (WeaponName == WeaponName4) return Weapon4Instance;

	return nullptr;
}

void APlayerCharacter::AirDash()
{
	if (bHasDashed || GetCharacterMovement()->IsMovingOnGround()) return;

	FVector InputVector = GetActorForwardVector() * InputComponent->GetAxisValue("MoveForward") +
		GetActorRightVector() * InputComponent->GetAxisValue("MoveRight");

	FVector DashDirection = InputVector.IsNearlyZero() ? GetActorForwardVector() : InputVector.GetClampedToMaxSize(1.0f);
	FVector DashVelocity = DashDirection * DashStrength;
	DashVelocity.Z = GetCharacterMovement()->JumpZVelocity;

	GetCharacterMovement()->Velocity = DashVelocity;
	bHasDashed = true;
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bHasDashed = false;
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0;

	PlayerHealth -= DamageAmount;
	if (PlayerHealth <= 0)
	{
		PlayerHealth = 0;
		bIsDead = true;
		DisableInput(nullptr);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		if (AArenaGameMode* GameMode = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->FadeOut(this);
			GameMode->PlayerDeath();
		}
	}

	return DamageAmount;
}
