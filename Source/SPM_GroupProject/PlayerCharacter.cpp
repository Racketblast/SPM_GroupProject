#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "Teleporter.h"
#include "Gun.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequencePlayer.h"
#include "ChallengeSubsystem.h"
#include "ArenaGameMode.h"
#include "Rifle.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;
	GetMesh()->SetupAttachment(PlayerCamera);
	
	SetupStimulusSource();
}

// Called when the game starts or when spawned
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
	
	if (UPlayerGameInstance *GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		//Adds a pistol if player does not have one each time player is loaded
		if (!GI->UpgradeMap.Contains(EUpgradeType::Pistol))
		{
			GI->UpgradeMap.Add(EUpgradeType::Pistol,GI->SetDefaultUpgradeInfo(EUpgradeType::Pistol));
			GI->SetCurrentWeapon(EUpgradeType::Pistol);
		}
		GI->ApplyAllUpgradeFunctions(this);
		SelectWeapon(GI->GetCurrentWeaponName());
	}
	
	PlayerHealth = PlayerMaxHealth;
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Weapon2Equipped && CurrentGun && CurrentGun->IsA<ARifle>() && bIsShooting)
	{
		Shoot();
	}

	// DrawDebugLine for use
	// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false);

	// Drawdebugline for socket
	/*const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
	FRotator SocketRot = SocketTransform.GetRotation().Rotator();
	FRotator SpawnRotation(SocketRot.Pitch + 12, GetActorRotation().Yaw, SocketRot.Roll);
	FVector SpawnLocation = SocketTransform.GetLocation();

	FVector EndLocation = SpawnLocation + SpawnRotation.Vector() * UseDistance;

	// Draw the debug line
	DrawDebugLine(
		GetWorld(),
		SpawnLocation,
		EndLocation,
		FColor::Green,
		false     // Thickness
	);*/

	
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &APlayerCharacter::Shoot);
	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &APlayerCharacter::Use);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::Reload);
	PlayerInputComponent->BindAction("SelectWeapon1", IE_Pressed, this, &APlayerCharacter::SelectWeapon1);
	PlayerInputComponent->BindAction("SelectWeapon2", IE_Pressed, this, &APlayerCharacter::SelectWeapon2);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Yaw", this, &APlayerCharacter::Yaw);
	PlayerInputComponent->BindAxis("Pitch", this, &APlayerCharacter::Pitch);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &APlayerCharacter::StartShooting);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &APlayerCharacter::StopShooting);
	
}
// Called when the player starts holding the shoot button
void APlayerCharacter::StartShooting()
{
	if (Weapon2Equipped && CurrentGun && CurrentGun->IsA<ARifle>())
	{
		bIsShooting = true; // Player starts shooting
	}
}

// Called when the player releases the shoot button
void APlayerCharacter::StopShooting()
{
	bIsShooting = false; // Player stops shooting
}
AGun* APlayerCharacter::GetCurrentGun() const
{
	return CurrentGun;
}


void APlayerCharacter::MoveForward(float Value)
{
	FVector ForwardDirection = GetActorForwardVector();
	AddMovementInput(ForwardDirection, Value);
}
void APlayerCharacter::MoveRight(float Value)
{
	FVector RightDirection = GetActorRightVector();
	AddMovementInput(RightDirection, Value);
}

void APlayerCharacter::Yaw(float Value)
{
	AddControllerYawInput(Value);
}
void APlayerCharacter::Pitch(float Value)
{
	AddControllerPitchInput(Value);
}

//Jag gör detta endast för att kunna kalla på en funktion från UChallengeSubsystem, om ni kommer på ett bättre sätt så kan ni bara byta till det. Dock borde jump fungera på samma sätt som förut.
void APlayerCharacter::Jump()
{
	// Notify Challenge Subsystem
	if (UChallengeSubsystem* ChallengeSub = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
	{
		ChallengeSub->NotifyPlayerJumped();
	}

	Super::Jump(); // Den faktiska jump funktionen 
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


void APlayerCharacter::Shoot()
{
	if (!CurrentGun)
		return;

	USceneComponent* Muzzle = CurrentGun->GetMuzzlePoint();
	if (!Muzzle)
		return;

	FVector FireLocation = Muzzle->GetComponentLocation();
	FRotator FireRotation = PlayerCamera->GetComponentRotation();

	CurrentGun->Fire(FireLocation, FireRotation);

	// Challenge system
	if (UChallengeSubsystem* ChallengeSubsystem = GetGameInstance()->GetSubsystem<UChallengeSubsystem>())
	{
		if (CurrentGun == Weapon1Instance)
		{
			ChallengeSubsystem->NotifyWeaponFired(WeaponName1);
		}
		else
		{
			ChallengeSubsystem->NotifyWeaponFired(WeaponName2);
		}
	}
}


void APlayerCharacter::Reload()
{
	if (CurrentGun) // Ensure the player has a current weapon
	{
		// Call the Reload function on the weapon
		CurrentGun->Reload();
		UE_LOG(LogTemp, Warning, TEXT("Player reload triggered"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped to reload"));
	}
}

void APlayerCharacter::SelectWeapon(FName Weapon)
{
	if (UPlayerGameInstance *GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (Weapon == WeaponName1)
		{
			APlayerCharacter::SelectWeapon1();
		}
		else if (Weapon == WeaponName2)
		{
			APlayerCharacter::SelectWeapon2();
		}
	}
}


void APlayerCharacter::SelectWeapon1()
{
	if (UPlayerGameInstance *PlayerGameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon1Equipped && PlayerGameInstance->HasBought(WeaponName1))
		{
			PlayerGameInstance->SetCurrentWeapon(WeaponName1);

			Weapon1Equipped = true;
			Weapon2Equipped = false;

			// Spawn the weapon if it's not already in the world
			if (!Weapon1Instance)
			{
				Weapon1Instance = GetWorld()->SpawnActor<AGun>(GWeapon1);
				if (Weapon1Instance)
				{
					Weapon1Instance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_rSocket"));
					Weapon1Instance->SetOwnerCharacter(this);

					// Example: Give Weapon1 infinite reloads
					Weapon1Instance->bHasInfiniteReloads = true;
				}
			}

			// Hide the other gun
			if (Weapon2Instance)
			{
				Weapon2Instance->SetActorHiddenInGame(true);
				Weapon2Instance->SetActorEnableCollision(false);
			}

			// Enable current gun
			if (Weapon1Instance)
			{
				Weapon1Instance->SetActorHiddenInGame(false);
				Weapon1Instance->SetActorEnableCollision(true);
				CurrentGun = Weapon1Instance;
			}
			CurrentGun->CheckForUpgrades();
		}
		
	}
}

void APlayerCharacter::SelectWeapon2()
{
	if (UPlayerGameInstance *PlayerGameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon2Equipped && PlayerGameInstance->HasBought(WeaponName2))
		{
			PlayerGameInstance->SetCurrentWeapon(WeaponName2);
			Weapon2Equipped = true;
			Weapon1Equipped = false;
			if (!Weapon2Instance)
			{
				Weapon2Instance = GetWorld()->SpawnActor<AGun>(GWeapon2);
				if (Weapon2Instance)
				{
					Weapon2Instance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_rSocket"));
					Weapon2Instance->SetOwnerCharacter(this);
				}
			}

			if (Weapon1Instance)
			{
				Weapon1Instance->SetActorHiddenInGame(true);
				Weapon1Instance->SetActorEnableCollision(false);
			}

			if (Weapon2Instance)
			{
				Weapon2Instance->SetActorHiddenInGame(false);
				Weapon2Instance->SetActorEnableCollision(true);
				CurrentGun = Weapon2Instance;
			}
			CurrentGun->CheckForUpgrades();
		}
	}
}

void APlayerCharacter::Use()
{
	const FVector Start = PlayerCamera->GetComponentLocation();
	const FVector End = Start + (PlayerCamera->GetForwardVector() * UseDistance);
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		if (AActor* TargetActor = HitResult.GetActor())
		{
			if (TargetActor && TargetActor->GetClass()->ImplementsInterface(UPlayerUseInterface::StaticClass()))
			{
				IPlayerUseInterface::Execute_Use(TargetActor, this);
			}
		}
		
	}
}

void APlayerCharacter::HealPlayer(int32 HealAmount)
{
	if (!bIsDead)
	{
		PlayerHealth += HealAmount;
		if (PlayerHealth > PlayerMaxHealth)
		{
			PlayerHealth = PlayerMaxHealth;
		}
	}
}

AGun* APlayerCharacter::GetWeaponInstance(const FName WeaponName) const
{
	if (WeaponName == "Pistol")
	{
		return Weapon1Instance;
	}
	if (WeaponName == "Rifle")
	{
		return Weapon2Instance;
	}
	if (WeaponName == "Shotgun")
	{
		return Weapon3Instance;
	}
		return nullptr;
}

float APlayerCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (!bIsDead)
	{
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
				if (GameMode->SequencePlayer)
				{
					GameMode->SequencePlayer->OnFinished.AddDynamic(GameMode, &AArenaGameMode::PlayerDeath);
				}
			}
		}
	}
	
	return DamageAmount;
}