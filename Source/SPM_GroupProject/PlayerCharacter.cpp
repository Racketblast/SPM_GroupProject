#include "PlayerCharacter.h"
#include "PlayerGameInstance.h"
#include "BuyBox.h"
#include "Teleporter.h"
#include "Projectile.h"
//#include "ProjectileSpawner.h"
#include "Gun.h"
#include "Engine/StaticMeshSocket.h"
#include "Kismet/GameplayStatics.h"

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
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
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
}


// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const FVector Start = PlayerCamera->GetComponentLocation();
	const FVector End = Start + (PlayerCamera->GetForwardVector() * UseDistance);
	//DrawDebugLine for use
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false);

	//Drawdebugline for socket
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
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &APlayerCharacter::Shoot);
	PlayerInputComponent->BindAction("Use", IE_Pressed,this, &APlayerCharacter::Use);
	PlayerInputComponent->BindAction("Reload", IE_Pressed,this, &APlayerCharacter::Reload);
	PlayerInputComponent->BindAction("SelectWeapon1", IE_Pressed,this, &APlayerCharacter::SelectWeapon1);
	PlayerInputComponent->BindAction("SelectWeapon2", IE_Pressed,this, &APlayerCharacter::SelectWeapon2);
	PlayerInputComponent->BindAxis("MoveForward",this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Yaw",this, &APlayerCharacter::Yaw);
	PlayerInputComponent->BindAxis("Pitch",this, &APlayerCharacter::Pitch);
	
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
	if (CurrentAmmo > 0)
	{
		
		if (Weapon1Equipped)
		{
			const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
			FRotator SocketRot = SocketTransform.GetRotation().Rotator();
			FRotator SpawnRotation( SocketRot.Pitch+12, GetActorRotation().Yaw,  SocketRot.Roll);
			GetWorld()->SpawnActor<AProjectile>(Projectile1, SocketTransform.GetLocation(), SpawnRotation);
		}
		if (Weapon2Equipped)
		{
			const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
			FRotator SocketRot = SocketTransform.GetRotation().Rotator();
			FRotator SpawnRotation( SocketRot.Pitch+12, GetActorRotation().Yaw,  SocketRot.Roll);
			GetWorld()->SpawnActor<AProjectile>(Projectile2, SocketTransform.GetLocation(), SpawnRotation);
		}
		CurrentAmmo--;
	}
}

void APlayerCharacter::Reload()
{
	if (Weapon2Equipped)
	{
		if (ExtraMags <= 0){
			return;
		}else
		{
			ExtraMags--;
		}
		
	}
	CurrentAmmo = CurrentMaxAmmo;
}

void APlayerCharacter::SelectWeapon1()
{
	if (UPlayerGameInstance* PlayerGameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon1Equipped && PlayerGameInstance->HasBought(WeaponName1))
		{
			if (Weapon2Equipped)
			{
				CurrentGun->Destroy();
				UE_LOG(LogTemp, Warning, TEXT("Gun Destroyed"));
			}
			PlayerGameInstance->SetCurrentWeapon(WeaponName1);
     
			if (CurrentMaxAmmo > 0)
			{
				Ammo2 = CurrentAmmo;
			}
     
			Weapon1Equipped = true;
			Weapon2Equipped = false;

			CurrentAmmo = Ammo1;
			CurrentMaxAmmo = MaxAmmo1;
     
			if (!GetMesh()->DoesSocketExist(TEXT("hand_lSocket")))
			{
				UE_LOG(LogTemp, Warning, TEXT("hand_lSocket not found on mesh."));
				return;
			}

			// Get the socket transform from the mesh
			const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
			AGun* SpawnedGun = GetWorld()->SpawnActor<AGun>(GWeapon1);
			if (SpawnedGun)
			{
				SpawnedGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_lSocket"));
				CurrentGun = SpawnedGun;
			}
		}
	}
}

void APlayerCharacter::SelectWeapon2()
{
	if (UPlayerGameInstance* PlayerGameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (!Weapon2Equipped && PlayerGameInstance->HasBought(WeaponName2))
		{if (Weapon1Equipped)
		{
			CurrentGun->Destroy();
			UE_LOG(LogTemp, Warning, TEXT("Gun Destroyed"));
		}
			PlayerGameInstance->SetCurrentWeapon(WeaponName2);
     
			if (CurrentMaxAmmo > 0)
			{
				Ammo1 = CurrentAmmo;
			}
     
			Weapon2Equipped = true;
			Weapon1Equipped = false;

			CurrentAmmo = Ammo2;
			CurrentMaxAmmo = MaxAmmo2;
     
			if (!GetMesh()->DoesSocketExist(TEXT("hand_lSocket")))
			{
				UE_LOG(LogTemp, Warning, TEXT("hand_lSocket not found on mesh."));
				return;
			}

			// Get the socket transform from the mesh
			const FTransform SocketTransform = GetMesh()->GetSocketTransform(TEXT("hand_lSocket"));
			AGun* SpawnedGun = GetWorld()->SpawnActor<AGun>(GWeapon2);
			if (SpawnedGun)
			{
				SpawnedGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_lSocket"));
				CurrentGun = SpawnedGun;
			}
		}
	}
}

//TODO: Make this a switch case and put it in it's own class
void APlayerCharacter::Use()
{
	if (TargetActor)
	{
		UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		//Teleporting function
		if (ATeleporter* Teleporter = Cast<ATeleporter>(TargetActor))
		{
			if (GI)
			{
				//If you can teleport
				//Checks if in wave and if you have the key
				if (!GI->bIsWave && GI->TeleportKeyArray[Teleporter->TeleportKeyNumber])
				{
					if (Teleporter->TargetLevelName != "Hub")
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), Teleporter->TeleportSound, Teleporter->GetActorLocation());
						GI->Level += 1;
						if (GI->TeleportKeyArray.IsValidIndex(GI->Level))
						{
							GI->TeleportKeyArray[GI->Level] = true;
						}
					}
					GI->Money += PickedUpMoney;
					
					UGameplayStatics::OpenLevel(this, Teleporter->TargetLevelName);
				}
				else
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), Teleporter->CantTeleportSound, Teleporter->GetActorLocation());
				}
			}
		}
		//Buying function
		if (const ABuyBox* BuyBox = Cast<ABuyBox>(TargetActor))
		{
			if (GI)
			{
				//If you don't own the product
				if (!GI->HasBought(BuyBox->TargetUpgradeName))
				{
					//If you can buy the product
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
					//If you can't buy the product
					else
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), BuyBox->CantBuySound, BuyBox->GetActorLocation());
					}
				}
				//If you own the product
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
}
