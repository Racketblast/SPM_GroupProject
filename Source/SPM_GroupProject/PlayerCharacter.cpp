#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "BuyBox.h"
#include "Teleporter.h"
#include "Gun.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;
	GetMesh()->SetupAttachment(PlayerCamera);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (UPlayerGameInstance *GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (GI->GetCurrentWeaponName() == WeaponName1)
		{
			APlayerCharacter::SelectWeapon1();
		}
		else if (GI->GetCurrentWeaponName() == WeaponName2)
		{
			APlayerCharacter::SelectWeapon2();
		}
		GI->GetAllUpgradeFunctions(this);
	}

	if (FadeInTransition)
	{
		if (TeleportInSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportInSound, GetActorLocation());
		}
		FMovieSceneSequencePlaybackSettings Settings;
		ALevelSequenceActor *OutActor;
		ULevelSequencePlayer *SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), FadeInTransition, Settings, OutActor);
		SequencePlayer->Play();
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector Start = PlayerCamera->GetComponentLocation();
	const FVector End = Start + (PlayerCamera->GetForwardVector() * UseDistance);
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

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		TargetActor = HitResult.GetActor();
	}
	else
	{
		TargetActor = nullptr;
	}
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
void APlayerCharacter::Shoot()
{
	if (!CurrentGun)
		return;
	UE_LOG(LogTemp, Warning, TEXT("Shoot function called"));
	const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
	FRotator SocketRot = SocketTransform.GetRotation().Rotator();
	FRotator FireRotation(SocketRot.Pitch + 20, GetActorRotation().Yaw, SocketRot.Roll);
	FVector FireLocation = SocketTransform.GetLocation();

	CurrentGun->Fire(FireLocation, FireRotation);
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
					Weapon1Instance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_lSocket"));
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
					Weapon2Instance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_lSocket"));
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
		}
	}
}

// TODO: Make this a switch case and put it in it's own class
void APlayerCharacter::Use()
{
	if (TargetActor)
	{
		UPlayerGameInstance *GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		// Teleporting function
		if (ATeleporter *Teleporter = Cast<ATeleporter>(TargetActor))
		{
			if (GI)
			{
				// If you can teleport
				// Checks if in wave and if you have the key
				// if (!GI->bIsWave && GI->TeleportKeyArray[Teleporter->TeleportKeyNumber])
				if (!GI->bIsWave && GI->UnlockedLevels.Contains(Teleporter->TargetLevelName))
				{
					/*if (Teleporter->TargetLevelName != "Hub")
					{
						GI->Level += 1;
						if (GI->TeleportKeyArray.IsValidIndex(GI->Level))
						{
							GI->TeleportKeyArray[GI->Level] = true;
						}
					}
					GI->Money += PickedUpMoney;*/

					UGameplayStatics::PlaySoundAtLocation(GetWorld(), Teleporter->TeleportSound, Teleporter->GetActorLocation());

					Teleporter->Teleport();
				}
				else
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), Teleporter->CantTeleportSound, Teleporter->GetActorLocation());
				}
			}
		}
	}
	// Buying function
	if (const ABuyBox *BuyBox = Cast<ABuyBox>(TargetActor))
	{
		if (GI)
		{
			// If you don't own the product
			if (!GI->HasBought(BuyBox->TargetUpgradeName))
			{
				// If you can buy the product
				if (BuyBox->TargetUpgradeCost <= GI->Money)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), BuyBox->BuySound, BuyBox->GetActorLocation());

					GI->Money -= BuyBox->TargetUpgradeCost;
					GI->UpgradeArray.Add(BuyBox->TargetUpgradeName);
					if (BuyBox->TargetUpgradeCategory == EUpgradeCategory::Weapon)
					{
						GI->SetCurrentWeapon(BuyBox->TargetUpgradeName);
						if (GI->GetCurrentWeaponName() == WeaponName1)
						{
							APlayerCharacter::SelectWeapon1();
						}
						else if (GI->GetCurrentWeaponName() == WeaponName2)
						{
							APlayerCharacter::SelectWeapon2();
						}
					}
					else
					{
						GI->GetSpecificUpgradeFunction(BuyBox->TargetUpgradeName, this);
					}
				}
				// If you can't buy the product
				else
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), BuyBox->CantBuySound, BuyBox->GetActorLocation());
				}
			}
			// If you own the product
			else
			{
				if (BuyBox->TargetUpgradeCategory == EUpgradeCategory::Weapon)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), BuyBox->BuySound, BuyBox->GetActorLocation());

					GI->SetCurrentWeapon(BuyBox->TargetUpgradeName);
					if (GI->GetCurrentWeaponName() == WeaponName1)
					{
						APlayerCharacter::SelectWeapon1();
					}
					else if (GI->GetCurrentWeaponName() == WeaponName2)
					{
						APlayerCharacter::SelectWeapon2();
					}
				}
				else
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), BuyBox->CantBuySound, BuyBox->GetActorLocation());
				}
			}
		}
	}
}
